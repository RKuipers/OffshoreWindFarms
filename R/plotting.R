#Import data
resultsRaw <- read.csv2(file = "aaa_Collective.csv")
colnames(resultsRaw) <- colnames(resultsRaw[-1])
resultsRaw <- subset(resultsRaw, select = -c(2, 15))

#Create Duration list
durList = list()
durList[unique(resultsRaw$Dur)] <- c(24, 6, 12, 21, 18, 33, 30)
conv <- function(dur) {return(durList$dur)}

#Set data category and total
library(dplyr)
resultsRaw <- resultsRaw %>% mutate(Category = ifelse(Turbines == 'Al', 'Sep', ifelse(Vessels == 'NO', 'Ove', 'Sha')))
dur.names = resultsRaw$Dur
resultsRaw$Dur <- unlist(durList[resultsRaw$Dur])
resultsRaw$Total <- (resultsRaw$Objective / resultsRaw$Dur) * 12
colnames(resultsRaw) <- c("Objective", "Avail time", "Avail ener", "ProdLoss", "DirCosts", "VesCosts", "RepCosts", "TechCosts", "Turbines", "Dur", "Vessels", "Percent", "Size", "Category", "Total")

#Plot
library(ggplot2)
library(GGally)
resultsAgg <- aggregate(cbind(Total, ProdLoss, DirCosts, VesCosts, RepCosts, TechCosts) ~ Category, resultsRaw, mean)
ggparcoord(resultsAgg, columns = c(2, 3, 4, 5, 6, 7), groupColumn = "Category", scale = "globalminmax") +
  geom_line(size = 1.5)

#Plot function
myplot <- function(df, group1 = "Category", group2 = NULL)
{
  ggparcoord(df, columns = c(3, 4, 5, 6, 7, 8), groupColumn = group1, scale = "globalminmax") +
    geom_line(size = 1) + 
    facet_wrap(group2, scales = "free")
}
library(parcoords)
myplot2 <- function(df, group1 = "Category", group2 = NULL)
{
  parcoords(df)
}

#Breakdown plots
library(crayon)
resultsDur <- aggregate(cbind(Total, ProdLoss, DirCosts, VesCosts, RepCosts, TechCosts) ~ Category + Dur, resultsRaw, mean)
ggparcoord(resultsDur, columns = c(3, 4, 5, 6, 7, 8), groupColumn = "Category", scale = "globalminmax") +
  geom_line(aes(lty = as.factor(Dur)), size = 1.5) + 
  scale_linetype_manual(values=c("twodash", "dotted", "longdash", "solid", "blank", "dotdash", "dashed"))
myplot(resultsDur, group2 = "Dur")

resultsTurb <- aggregate(cbind(Total, ProdLoss, DirCosts, VesCosts, RepCosts, TechCosts) ~ Category + Turbines, resultsRaw, mean)
ggparcoord(resultsTurb, columns = c(3, 4, 5, 6, 7, 8), groupColumn = "Category", scale = "globalminmax") +
  geom_line(aes(lty = as.factor(Turbines)), size = 1.5) + 
  scale_linetype_manual(values=c("twodash", "dotted", "longdash", "solid", "blank", "dotdash", "dashed"))
myplot(resultsTurb, group2 = "Turbines")

resultsVes <- aggregate(cbind(Total, ProdLoss, DirCosts, VesCosts, RepCosts, TechCosts) ~ Category + Vessels, resultsRaw, mean)
#ggparcoord(resultsVes, columns = c(4, 5, 6, 7, 8), groupColumn = "Category", scale = "globalminmax") +
  #geom_line(aes(lty = as.factor(Vessels)), size = 1.5) + 
  #scale_linetype_manual(values=c("twodash", "dotted", "longdash", "solid", "dotdash", "dashed"))
myplot(resultsVes)

resultsPerc <- aggregate(cbind(Total, ProdLoss, DirCosts, VesCosts, RepCosts, TechCosts) ~ Category + Percent, resultsRaw, mean) %>% transform(Percent = chr(Percent))
#ggparcoord(resultsPerc, columns = c(4, 5, 6, 7, 8), groupColumn = "Category", scale = "globalminmax") +
  #geom_line(aes(lty = as.factor(Percent)), size = 1.5) + 
  #scale_linetype_manual(values=c("twodash", "dotted", "longdash", "solid", "dotdash", "dashed"))
myplot(resultsPerc)

resultsSize <- aggregate(cbind(Total, ProdLoss, DirCosts, VesCosts, RepCosts, TechCosts) ~ Category + Size, resultsRaw, mean)
myplot(resultsSize, "Category", "Size")
parcoords(resultsSize, rownames = F, color = list(colorScale = "scaleOrdinal", colorBy = "Category", colorScheme = "schemeCategory10"), withD3 = TRUE)

#TODO: Check out parcoords ipv ggparcoord
#TODO: Create "normalized" dataframe which normalizes all data by #turbines
#TODO: Make 2 plot functions: plotfacet (what I currently have, though can be extended with 3rd group (2nd facet)) and plotline (or other method to visualize second split) 
