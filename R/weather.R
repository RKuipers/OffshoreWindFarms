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
library(dplyr)
waveheight <- subset(read.table("FINO1_waveheight.dat", F, "\t", skip = 6), select = c(1, 2))
colnames(waveheight) <- c("Time", "Value")
waveheight$Time <- strptime(waveheight$Time, form)
start = waveheight[1,1]

getHour <- function(x) { 
  floor(as.numeric(x - start, units="hours"))}

#Combine weather data
weather <- aggregate(Value ~ Hour + Month, mutate(waveheight, Hour = getHour(Time), Month = Time$mon), max)
colnames(weather)[3] <- "MaxWave"
windspeed$Time <- strptime(windspeed$Time, form)
weather <- full_join(weather, aggregate(Value ~ Hour + Month, subset(mutate(windspeed, Hour = getHour(Time), Month = Time$mon), Hour >= 0), max))
colnames(weather)[4] <- "MaxWind"

#Aggregate and clean data
weather$MaxWave[weather$MaxWave == -999] <- NA
weather$MaxWind[weather$MaxWind == -999] <- NA
weather <- mutate(weather, Allowed1 = MaxWave <= 1.5, Allowed2 = (MaxWave <= 2.0 | is.na(MaxWave)) & (MaxWind <= 10.0 | is.na(MaxWind)))
weather$Allowed2[is.na(weather$MaxWave) & is.na(weather$MaxWind)] <- NA
allowedPercentage <- aggregate(cbind(Allowed1, Allowed2) ~ Month, weather, mean)
print(allowedPercentage)