import R;

// R function to calculate sse from SEIR model full output
global const string template =
"""
colnum <- %s
data <- read.csv(text = "%s", header = F)

flu_data <- as.numeric(unlist(strsplit( "%s", split = ",")))
res <- sum( (data[-1,colnum] - (flu_data * %s) )^2)
""";

(string result) calc_obj (string str_data, string str_flu_data, int col_num, float scaling = 1)
{
    string code = template % (fromint(col_num), str_data, str_flu_data, fromfloat(scaling));
    result = R(code,"toString(res)");
}


global const string flu_template =
"""
col_name <- make.names("%s")
data <- read.csv(text = "%s")
res1 <- data[,col_name]
res <- paste(res1,collapse = ",")
""";
(string result) get_flu_data(string all_flu_data, int flu_year){
    string code = flu_template % (fromint(flu_year),all_flu_data);
    result = R(code,"toString(res)");
}
