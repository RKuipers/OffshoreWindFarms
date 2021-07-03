#Import data
resultsRaw <- read.csv2(file = "aaa_Collective.csv")
colnames(resultsRaw) <- colnames(resultsRaw[-1])
resultsRaw <- subset(resultsRaw, select = -15)

#Set data category
library(plyr)
resultsRaw <- resultsRaw %>% mutate(Category = ifelse(Turbines == 'Al', 'Sep', ifelse(Vessels == 'NO', 'Ove', 'Sha')))
results <- subset(resultsRaw, select = -c(2, 3, 4, 10, 11, 12, 13, 14, 15))
resultsAgg <- aggregate(results, resultsRaw[15], mean)

#Plot
library(ggplot2)
library(GGally)
ggparcoord(resultsAgg, columns = 2:ncol(resultsAgg), scale = "center")
#TODO: Determine scaling (y axis)
#TODO: Colour lines to differentiate (through mapping parameter, aes string), thicker lines
#TODO: Make plots for the 5 different model parameters, while coloring based on the 3 categories