#include "omp.h"

#include "repast_hpc/io.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/initialize_random.h"

#include "boost/tokenizer.hpp"
#include "boost/algorithm/string.hpp"

#include "chi_sim/Parameters.h"

#include "crx_person_data.h"
#include "CRXModel.h"
#include "CRXRandom.h"
#include "run.h"
#include "parameter_constants.h"
#include "utils.h"

namespace crx {

void parse_parameters(int rank, repast::Properties& props, const std::string& parameters) {
    boost::char_separator<char> comma_sep(",");
    boost::tokenizer<boost::char_separator<char> > comma_tok(parameters, comma_sep);

    for (auto item : comma_tok) {

        size_t pos = item.find_first_of("=");
        if (pos == std::string::npos) {
            throw invalid_argument("Invalid parameter: " + item);
        }

        string key(item.substr(0, pos));
        boost::trim(key);
        if (key.length() == 0) {
            throw invalid_argument("Invalid parameter: " + item);
        }

        string val(item.substr(pos + 1, item.length()));
        boost::trim(val);
        if (val.length() == 0) {
            throw invalid_argument("Invalid parameter: " + item);
        }

        props.putProperty(key, val);
    }
}

void run_model(repast::Properties& props) {
    boost::mpi::communicator comm = *(repast::RepastProcess::instance()->getCommunicator());
    int num_threads;

    #pragma omp parallel
    {
        num_threads = omp_get_num_threads();
    }

    if (comm.rank() == 0) {
        
        std::cout << "Run Start Mem: " << get_mem() << std::endl;
        //double vm, rss;
        //process_mem_usage(vm, rss);
        //cout << "VM: " << vm << "; RSS: " << (rss / 1024.0) << endl;

        //if (thread_num == 0) {
            std::cout << "num threads: " << num_threads << std::endl;
        //}

        std::cout << "World Size: " << comm.size() << std::endl;
        std::string time;
        repast::timestamp(time);
        std::cout << "Start Time: " << time << std::endl;
    }

    // initialize repast's rng
    initializeRandom(props);
    boost::uint32_t seed = repast::Random::instance()->seed();
    CRXRandom::initialize(seed, num_threads);

    
    MPI_Datatype dt;
    crx::make_mpi_person_type(&dt);
    // chi_sim AbstractModel will fill the parameters from props
    crx::CRXModel model(props, dt);

    if (comm.rank() == 0) {
        std::string time;
        repast::timestamp(time);
        std::cout << "Schedule Start Time: " << time << std::endl;
    }


    props.writeToPropsFile(props.getProperty(OUTPUT_DIRECTORY) + "/parameters.txt", "");

    repast::RepastProcess::instance()->getScheduleRunner().run();

    if (comm.rank() == 0) {
        std::string time;
        repast::timestamp(time);
        std::cout << "End Time: " << time << std::endl;
    }
}

std::string crx_model_run(MPI_Comm comm, const std::string& props_file, const std::string& parameters) {
    char arg0[] = "main";
    char* argv[] = { &arg0[0], nullptr };
    int argc = (int) (sizeof(argv) / sizeof(argv[0])) - 1;
    // need the tmp in here because environment takes a reference
    char** tmp = &argv[0];
    boost::mpi::environment env(argc, tmp);

    std::string ret;
    if (parameters.size() == 0) {
        ret = "";
    } else {
        boost::mpi::communicator boost_comm(comm, boost::mpi::comm_attach);

        repast::Properties props(props_file);
        parse_parameters(boost_comm.rank(), props, parameters);

        //std::cout << rank << ": " << props.getProperty("incubation.duration.max") << std::endl;
        repast::RepastProcess::init("", &boost_comm);
        run_model(props);
        repast::RepastProcess::instance()->done();
        ret = props.getProperty(OUTPUT_DIRECTORY) + "/" + props.getProperty(VISIT_OUTPUT_FILE);
        if (boost_comm.rank() == 0) {
            std::cout << "Run End Mem: " << get_mem() << std::endl;
        }
    }
    return ret;
}

}
