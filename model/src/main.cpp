#include "omp.h"

#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/io.h"

#include "CRXModel.h"
#include "constants.h"
#include "crx_person_data.h"
#include "run.h"

using namespace repast;

void usage() {
    std::cerr << "usage: X  model_properties_file" << std::endl;
    std::cerr << "\tmodel_properties_file: the path to the model properties file" << std::endl;
}

// void runModel(std::string propsFile, int argc, char** argv) {
//     boost::mpi::communicator comm;
//     if (comm.rank() == 0) {


// #pragma omp parallel
//         {
//             int thread_num = omp_get_thread_num();
//             if (thread_num == 0) {
//                 std::cout << "num threads: " << omp_get_num_threads() << std::endl;
//             }
//         }

//         std::cout << "World Size: " << comm.size() << std::endl;
//         std::string time;
//         repast::timestamp(time);
//         std::cout << "Start Time: " << time << std::endl;
//     }

//     Properties props(propsFile, argc, argv);
//     initializeRandom(props);

//     MPI_Datatype dt;
//     crx::make_mpi_person_type(&dt);
//     // chi_sim AbstractModel will fill the parameters from props
//     crx::CRXModel model(props, dt);

//     if (comm.rank() == 0) {
//         std::string time;
//         repast::timestamp(time);
//         std::cout << "Schedule Start Time: " << time << std::endl;
//     }

//     props.writeToPropsFile("./parameters.txt", "");


//     RepastProcess::instance()->getScheduleRunner().run();

//     if (comm.rank() == 0) {
//         std::string time;
//         repast::timestamp(time);
//         std::cout << "End Time: " << time << std::endl;
//     }
// }

int main(int argc, char **argv) {
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    // if there aren't enough arguments, warn the user and exit
    if (argc < 2) {
        usage();
        return -1;
    }

    // model props file
    std::string props_file = argv[1];
    if (props_file.size() > 0) {
        try {
            RepastProcess::init("", &world);
            Properties props(props_file, argc, argv);
            crx::run_model(props);
            RepastProcess::instance()->done();

        } catch (std::exception& ex) {
            std::cerr << "Error while running the model: " << ex.what() << std::endl;
            throw ex;
        }
    } else {
        if (world.rank() == 0)
            usage();
    }

    return 0;
}
