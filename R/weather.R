form <- "%Y-%m-%d %H:%M:%S"

#Get windspeed data
windspeed <- read.table("FINO1_windspeed.dat", F, "\t", skip = 6)
colnames(windspeed) <- c("Time", "Value", "Minimum", "Maximum", "Deviation", "Quality")

#Get power/value curve
curveP <- c(0, 0, 0, 75, 187, 348, 574, 875, 1257, 1688, 2118, 2514, 2817, 2958, 2994, 2999, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000)
curve <- approxfun(curveP * 0.09, yleft = 0, yright = 0)

#Calculate average wind speeds
windspeedValues <- data.frame(strptime(windspeed$Time, form)$mon, curve(windspeed$Value))
colnames(windspeedValues) <- c("month", "value")
windspeedMeans <- aggregate(value ~ month, windspeedValues, mean)
print(windspeedMeans$value)

#Get waveheight data
waveheight <- subset(read.table("FINO1_waveheight.dat", F, "\t", skip = 6), select = c(1, 2))
colnames(waveheight) <- c("Time", "Value")
waveheight$Time <- strptime(waveheight$Time, form)

#TODO: Break Time down to 1hr intervals, get average (or max?) heights within interval (for both height and speed)
#TODO: Based on windspeed and waveheight, give each 1hr interval a category (all ships, mid ships, no ships)
#TODO: For each month count what % of time each ship can be active