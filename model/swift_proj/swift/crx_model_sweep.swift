import io;
import sys;
import files;
import R;
import python;
import string;

import crx_model;

string emews_root = getenv("EMEWS_PROJECT_ROOT");
string turbine_output = getenv("TURBINE_OUTPUT");
int procs_per_run = toint(getenv("PROCS_PER_RUN"));

string config_file = argv("config_file");

(void v) run_model()
{
	string param_file = argv("f"); // e.g. -f="model_params.txt"
	string param_lines[] = file_lines(input(param_file));

	string zs[];
  foreach pl,i in param_lines {
    //string beta_output = "beta.output.file = %s/run_%d/beta_scores.csv" % (turbine_output, i);
    //string choice_output = "choice.output.file = %s/run_%d/choices.csv" % (turbine_output, i);
    //string visit_output = "visit.output.file = %s/run_%d/visits.csv" % (turbine_output, i);
    //string dosing_output = "dosing.output.file = %s/run_%d/dosing.csv" % (turbine_output, i);
    string output = "output.directory = %s/run_%d" % (turbine_output, i);

    //string defaults = "%s\t%s\t%s\t%s\t%s" % (beta_output, choice_output, visit_output, dosing_output,
    // output);
    string line = "%s,%s" % (output, pl);
		zs[i] = @par=procs_per_run crx_model_run(config_file, line);
  }

	//results_file = "%s/results.csv" % (turbine_output);
  //file out<results_file> = write(join(zs, "\n") + "\n");
	v = propagate(size(zs));
}

main {
	run_model() =>
	@par=procs_per_run crx_model_run("", "");
}
