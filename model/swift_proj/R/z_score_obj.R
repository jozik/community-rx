library(data.table)

calc_class <- function(visits.path, target.path) {
  target <- fread(target.path)
  target <- target[Target.Mean > 20]
  target.clinics <- target[,unique(place_id)]
  
  target$place_id <- as.factor(target$place_id)
  setkey(target,place_id)
  
  z.scores <- ZS(visits.path, target.clinics, target)
  print(z.scores)
  good.places <- z.scores[week == 3 & abs(z) <= 4, place_id]
  cls <- ifelse(length(good.places) == length(target.clinics), "X1", "X0")
}


ZS <- function(visits.path,target.clinics,target){
  require(data.table)
  visits <-fread(visits.path) 
  ### List of service codes of interest
  service.codes <- c("G05", "G07", "G02", "G12", "G14")
  # create week column
  visits[,week := ceiling(tick / (7*24))]
  # Agg. Visit stats by place_id and week, for target clinics, select svc codes, people in choices file making choice==1, and 080401 only
  dt <- visits[place_id %in% target.clinics & service_code %in% service.codes & atus_code == 80401, .N, by = c("place_id","week")]
  dim(dt)
  # make the place_id column a factor
  dt$place_id <- as.factor(dt$place_id)
  # Calculating z score
  z.score <- function(x,mu,sd){
    (x-mu)/sd
  }
  # merge target clincis, targets and output
  x <- merge(target,dt, all.y=TRUE)
  # ignoring NAs
  x <- x[!is.na(x$Target.Mean)]
  # calculating z scores for all values.
  x$z <- z.score(x$N,x$Target.Mean,x$Target.SD)  
  return(x)
}
