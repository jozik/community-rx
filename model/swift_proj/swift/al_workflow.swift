import io;
import sys;
import files;
import location;
import string;
import EQR;
import crx_model;
import R_obj;

string emews_root = getenv("EMEWS_PROJECT_ROOT");
string resident_work_ranks = getenv("RESIDENT_WORK_RANKS");
string turbine_output = getenv("TURBINE_OUTPUT");
int procs_per_run = toint(getenv("PROCS_PER_RUN"));


string r_ranks[] = split(resident_work_ranks,",");
string algorithm = strcat(emews_root,"/R/algo.R");
string param_set = argv("param_set");
int num_clusters        = toint(argv("num_clusters"));
int num_random_sampling = toint(argv("num_random_sampling"));
int max_iter = toint(argv("max_iter", "2"));
int n = toint(argv("n"));
int trials = toint(argv("trials", "20"));
string restart_file = argv("restart_file", "");

printf("restart file '%s'", restart_file);

string config_file = argv("config_file");
string obj_r_file = strcat(emews_root, "/R/z_score_obj.R");
string targets_file = argv("targets_file");

// params passed into algorithm from param set file:
// dosing.decay, dosing.peer, gamma.med, propensity.multiplier, delta.multiplier
string input_template = "random.seed=%d,output.directory=%s,dosing.decay=%f,dosing.peer=%f,gamma.med=%f,propensity.multiplier=%f,delta.multiplier=%f";

string algo_params = """
  data_file = "%s",
  data_cols = 1:5,
  n = %i,
  num_folds = 3,
  max_iter = %i,
  # clustering thresholds
  low_thresh = 0.20,
  high_thresh = 0.80,
  num_cluster_sampling = %i,
  max_clusters = %i,
  num_random_sampling = %i,
  random_sampling_decrease = 0,
  target_metric = "fscore",
  target_metric_value = 0.99,
  ntree = 20,
  restart_file = "%s",
  outdir = "%s"
""" % (param_set, n, max_iter, num_clusters, num_clusters, num_random_sampling,
  restart_file, turbine_output);

string obj_template =
"""
# path to R/z_score_obj.R
source('%s')
# visits.path, target.path
# visits.path: from model
# target.path: /home/nick/Documents/crx_input/common/visitsZscores_filtered.csv
cls <- calc_class('%s', '%s')
""";

(string combos[]) create_parameter_combinations(string params, int trials) {
  foreach i in [0:trials-1:1] {
    combos[i] = fromint(i) + "," + params;
  }
}

(string cls) run_model(string params, int param_i, int iter) {
    string results[];
    string parameter_combos[] = create_parameter_combinations(params, trials);
    foreach s,i in parameter_combos {
      string param_vals[] = split(s, ",");
      //int run_num = iter * trials + i;
      string output = "%s/run_%d_%d_%d" % (turbine_output, i, param_i, iter);
      // "random.seed=%d,output.directory=%s,dosing.decay=%f,dosing.peer=%f,gamma.med=%f,propensity.multiplier=%f,delta.multiplier=%f";
      string line = input_template % (string2float(param_vals[0]), output, string2float(param_vals[1]),
        string2float(param_vals[2]), string2float(param_vals[3]), string2float(param_vals[4]), string2float(param_vals[5]));
      printf("params: %s", line);
      visits_file = @par=procs_per_run crx_model_run(config_file, line);
      string code = obj_template % (obj_r_file, visits_file, targets_file);
  		results[i] = R(code, "cls");
    }

    //string result = string_join(results, "\n");
    //string code = result_template % result;
    //cls = R(code, "toString(res)");

    // doing a single iteration so cls is results[0]
    cls = results[0];
}

() print_time (string id) "turbine" "0.0" [
  "puts [concat \"@\" <<id>> \" time is: \" [clock milliseconds]]"
];

(void o) al (int r_rank, int random_seed) {
    location loc = locationFromRank(r_rank);
    EQR_init_script(loc, algorithm) =>
    EQR_get(loc) =>
    EQR_put(loc, algo_params) =>
    doAL(loc,random_seed) => {
        EQR_stop(loc);
        o = propagate();
    }
}

(void v) doAL (location loc, int random_seed) {

    for (boolean b = true, int i = 1;
       b;
       b=c, i = i + 1)
  {
    string params =  EQR_get(loc);
    //printf("Iter %i  next params: %s", i, params);
    printf("Iter %i", i);
    boolean c;
    if (params == "FINAL") {
        string final_results =  EQR_get(loc);
        printf("Final results: %s", final_results) =>
        @par=procs_per_run crx_model_run("", "") =>
        v = make_void() =>
        c = false;
    } else if (params == "EQR_ABORT") {
      printf("EQR aborted: see output for R error") =>
      string why = EQR_get(loc);
      printf("%s", why) =>
      v = propagate() =>
      c = false;
    } else {
      string param_array[] = split(params, ";");
      string results[];
      foreach p, j in param_array
      {
          results[j] = run_model(p, j, i);
      }

      string res = join(results, ";");
      EQR_put(loc, res) => c = true => print_time(fromint(i));
    }
  }
}

printf("WORKFLOW!");

printf("algorithm: %s", algorithm);
al(toint(r_ranks[0]),0);
