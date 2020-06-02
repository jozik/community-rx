/*
 * recording_tests.cpp
 *
 *  Created on: Dec 15, 2017
 *      Author: nick
 */
#include <vector>

#include "boost/filesystem.hpp"

#include "gtest/gtest.h"
#include "mpi.h"

#include "chi_sim/CSVReader.h"

#include "../src/ConcatenatingDataRecorder.h"
#include "../src/Stats.h"
#include "../src/Statistics.h"

/**
 * Run with mpirun -n 3
 */

using namespace crx;

TEST(RecordingTests, testBetas) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    MPI_Datatype beta_type;
    make_beta_type(&beta_type);

    std::vector<crx::BetaScore> expected { { 0.1, 3.5, 2, 0 }, { 2.1, 4.5, 1, 3 }, { 3.1, 4.1, 100,
            12 }, { 15.5, 22.2, 343, 12 }, { 2, 1, 3434, 123 }, { 342.1, 43, 3345, 6 } };

    if (rank == 0) {
        std::vector<crx::BetaScore> my_scores(expected.begin(), expected.begin() + 1);
        std::vector<int> counts(world_size, 0);
        int size = my_scores.size();
        MPI_Gather(&size, 1, MPI_INT, &counts[0], 1, MPI_INT, 0, MPI_COMM_WORLD);
        ASSERT_EQ(1, counts[0]);
        ASSERT_EQ(2, counts[1]);
        ASSERT_EQ(3, counts[2]);

        int* displacements = new int[world_size];
        displacements[0] = 0;
        for (size_t i = 1; i < counts.size(); ++i) {
            displacements[i] = displacements[i - 1] + counts[i - 1];
        }

        int exp_count = std::accumulate(counts.begin(), counts.end(), 0);
        std::vector<crx::BetaScore> scores;
        scores.resize(exp_count);
        MPI_Gatherv(&my_scores[0], size, beta_type, &scores[0], &counts[0], displacements,
                beta_type, 0, MPI_COMM_WORLD);

        ASSERT_EQ(expected.size(), exp_count);

        for (size_t i = 0; i < expected.size(); ++i) {
            ASSERT_EQ(expected[i].tick, scores[i].tick);
            ASSERT_EQ(expected[i].beta, scores[i].beta);
            ASSERT_EQ(expected[i].person_id, scores[i].person_id);
            ASSERT_EQ(expected[i].place_id, scores[i].place_id);
        }

    } else if (rank == 1) {
        std::vector<crx::BetaScore> my_scores(expected.begin() + 1, expected.begin() + 3);
        int size = my_scores.size();
        MPI_Gather(&size, 1, MPI_INT, nullptr, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gatherv(&my_scores[0], size, beta_type, nullptr, nullptr, nullptr, beta_type, 0,
        MPI_COMM_WORLD);
    } else if (rank == 2) {
        std::vector<crx::BetaScore> my_scores(expected.begin() + 3, expected.end());
        int size = my_scores.size();
        MPI_Gather(&size, 1, MPI_INT, nullptr, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gatherv(&my_scores[0], size, beta_type, nullptr, nullptr, nullptr, beta_type, 0,
        MPI_COMM_WORLD);
    }

    MPI_Type_free(&beta_type);
}

TEST(RecordingTests, testDataRecorder) {
    std::string fname("../test_data/out.txt");
    boost::filesystem::path filepath(fname);
    if (boost::filesystem::exists(filepath)) {
        boost::filesystem::remove(filepath);
    }

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    crx::ConcatenatingDataRecorder<crx::BetaScore> writer(fname, BETA_HEADER, rank, make_beta_type);

    std::vector<crx::BetaScore> expected { { 0.1, 3.5, 2, 0 }, { 2.1, 4.5, 1, 3 }, { 3.1, 4.1, 100,
            12 }, { 15.5, 22.2, 343, 12 }, { 2, 1, 3434, 123 }, { 342.1, 43, 3345, 6 } };

    if (rank == 0) {
        std::vector<crx::BetaScore> my_scores(expected.begin(), expected.begin() + 1);
        for (auto i : my_scores) {
            writer.record(i);
        }
        writer.write(write_beta_type);
    } else if (rank == 1) {
        std::vector<crx::BetaScore> my_scores(expected.begin() + 1, expected.begin() + 3);
        for (auto i : my_scores) {
            writer.record(i);
        }
        writer.write(write_beta_type);
    } else {
        std::vector<crx::BetaScore> my_scores(expected.begin() + 3, expected.end());
        for (auto i : my_scores) {
            writer.record(i);
        }
        writer.write(write_beta_type);
    }
    writer.close();

    if (rank == 0) {
        ASSERT_TRUE(boost::filesystem::exists(filepath));

        chi_sim::CSVReader reader(fname);
        std::vector<std::string> line;
        int i = 0;
        // skip header
        reader.next(line);
        while (reader.next(line)) {
            ASSERT_EQ(4, line.size());
            ASSERT_EQ(expected[i].tick, std::stod(line[0]));
            ASSERT_EQ(expected[i].person_id, std::stol(line[1]));
            ASSERT_EQ(expected[i].place_id, std::stoi(line[2]));
            ASSERT_EQ(expected[i].beta, std::stod(line[3]));
            ++i;
        }
        ASSERT_EQ(expected.size(), i);
    }
}

ResourceVisit create_resource_visit(double tick, unsigned int p_id, int place_id, const std::string& code, int atus_code) {
    ResourceVisit visit;
    visit.tick = tick;
    visit.person_id = p_id;
    visit.place_id = place_id;
    visit.atus_code = atus_code;
    std::strcpy(visit.service_code, code.c_str());
    return visit;
}

TEST(RecordingTests, testVisitsRecorder) {
    std::string fname("../test_data/visits.csv");
    boost::filesystem::path filepath(fname);
    if (boost::filesystem::exists(filepath)) {
        boost::filesystem::remove(filepath);
    }

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    crx::ConcatenatingDataRecorder<crx::ResourceVisit> writer(fname, RESOURCE_VISIT_HEADER, rank, make_resource_vist_type);

    /*
     * struct ResourceVisit {
    double tick;
    unsigned int person_id;
    int place_id;
    // max 3 + null terminator
    char service_code[4];
    unsigned int atus_code;
};
     */
    std::vector<crx::ResourceVisit> expected { create_resource_visit( 0.1, 3, 12, "A01",  101 ),  create_resource_visit( 3, 1, 40, "H",  10101 ),
        create_resource_visit( 12, 13, 42, "H01",  12 ),  create_resource_visit( 3, 1, 40, "S01",  134 ), create_resource_visit(1.1, 24, 411, "I01",  343 ),
        create_resource_visit( 343.1, 3434, 400001, "H11",  120000 ) };

    if (rank == 0) {
        std::vector<crx::ResourceVisit> my_scores(expected.begin(), expected.begin() + 1);
        for (auto i : my_scores) {
            writer.record(i);
        }
        writer.write(write_resource_visit_type);
    } else if (rank == 1) {
        std::vector<crx::ResourceVisit> my_scores(expected.begin() + 1, expected.begin() + 3);
        for (auto i : my_scores) {
            writer.record(i);
        }
        writer.write(write_resource_visit_type);
    } else {
        std::vector<crx::ResourceVisit> my_scores(expected.begin() + 3, expected.end());
        for (auto i : my_scores) {
            writer.record(i);
        }
        writer.write(write_resource_visit_type);
    }
    writer.close();

    if (rank == 0) {
        ASSERT_TRUE(boost::filesystem::exists(filepath));

        chi_sim::CSVReader reader(fname);
        std::vector<std::string> line;
        int i = 0;
        // skip header
        reader.next(line);
        while (reader.next(line)) {
            ASSERT_EQ(5, line.size());
            ASSERT_EQ(expected[i].tick, std::stod(line[0]));
            ASSERT_EQ(expected[i].person_id, std::stol(line[1]));
            ASSERT_EQ(expected[i].place_id, std::stoi(line[2]));
            ASSERT_EQ(expected[i].service_code, line[3]);
            ASSERT_EQ(expected[i].atus_code, std::stoi(line[4]));
            ++i;
        }
        ASSERT_EQ(expected.size(), i);
    }
}

TEST(RecordingTests, testStats) {
    std::string beta_fname("../test_data/beta_out.txt");
    boost::filesystem::path beta_filepath(beta_fname);
    if (boost::filesystem::exists(beta_filepath)) {
        boost::filesystem::remove(beta_filepath);
    }

    std::string choice_fname("../test_data/choice_out.txt");
    boost::filesystem::path choice_filepath(choice_fname);
    if (boost::filesystem::exists(choice_filepath)) {
        boost::filesystem::remove(choice_filepath);
    }

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    ResourceVisitRecorder recorder("/dev/null", 0);
    Statistics* stats = Statistics::initialize(beta_fname, choice_fname, recorder, rank);

    std::vector<crx::BetaScore> beta_expected { { 0.1, 3.5, 2, 0 }, { 2.1, 4.5, 1, 3 }, { 3.1, 4.1,
            100, 12 }, { 15.5, 22.2, 343, 12 }, { 2, 1, 3434, 123 }, { 342.1, 43, 3345, 6 } };

    std::vector<crx::ChoiceEntry> choice_expected { { 0.1, 2, 0 }, { 2.1, 1, 1 }, { 3.1, 1, 0 }, {
            15.5, 12, 0 }, { 2, 343, 1 }, { 342.1, 3345, 1 } };

    if (rank == 0) {
        std::vector<crx::BetaScore> my_betas(beta_expected.begin(), beta_expected.begin() + 1);
        for (auto i : my_betas) {
            stats->recordBetaScore(i.tick, i.person_id, i.place_id, i.beta);
        }

        std::vector<crx::ChoiceEntry> my_choices(choice_expected.begin(),
                choice_expected.begin() + 2);
        for (auto i : my_choices) {
            stats->recordChoice(i.tick, i.person_id, i.choice);
        }

    } else if (rank == 1) {
        std::vector<crx::BetaScore> my_betas(beta_expected.begin() + 1, beta_expected.begin() + 3);
        for (auto i : my_betas) {
            stats->recordBetaScore(i.tick, i.person_id, i.place_id, i.beta);
        }

        std::vector<crx::ChoiceEntry> my_choices(choice_expected.begin() + 2,
                choice_expected.begin() + 3);
        for (auto i : my_choices) {
            stats->recordChoice(i.tick, i.person_id, i.choice);
        }

    } else {
        std::vector<crx::BetaScore> my_betas(beta_expected.begin() + 3, beta_expected.end());
        for (auto i : my_betas) {
            stats->recordBetaScore(i.tick, i.person_id, i.place_id, i.beta);
        }

        std::vector<crx::ChoiceEntry> my_choices(choice_expected.begin() + 3,
                choice_expected.end());
        for (auto i : my_choices) {
            stats->recordChoice(i.tick, i.person_id, i.choice);
        }
    }
    stats->writeBetaScores();
    stats->writeChoices();

    if (rank == 0) {
        ASSERT_TRUE(boost::filesystem::exists(beta_filepath));

        chi_sim::CSVReader reader(beta_fname);
        std::vector<std::string> line;
        int i = 0;
        // skip header
        reader.next(line);
        while (reader.next(line)) {
            ASSERT_EQ(4, line.size());
            ASSERT_EQ(beta_expected[i].tick, std::stod(line[0]));
            ASSERT_EQ(beta_expected[i].person_id, std::stol(line[1]));
            ASSERT_EQ(beta_expected[i].place_id, std::stoi(line[2]));
            ASSERT_EQ(beta_expected[i].beta, std::stod(line[3]));
            ++i;
        }

        ASSERT_EQ(beta_expected.size(), i);

        ASSERT_TRUE(boost::filesystem::exists(choice_filepath));
        i = 0;
        chi_sim::CSVReader reader1(choice_fname);
        std::vector<std::string> line1;
        // skip header
        reader1.next(line1);
        while (reader1.next(line1)) {
            ASSERT_EQ(3, line1.size());
            ASSERT_EQ(choice_expected[i].tick, std::stod(line1[0]));
            ASSERT_EQ(choice_expected[i].person_id, std::stol(line1[1]));
            ASSERT_EQ(choice_expected[i].choice, std::stoi(line1[2]));
            ++i;
        }

        ASSERT_EQ(choice_expected.size(), i);
    }
}

