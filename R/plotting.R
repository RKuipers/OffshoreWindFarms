#Import data
resultsRaw <- read.csv2(file = "aaa_Collective.csv")
colnames(resultsRaw) <- colnames(resultsRaw[-1])
resultsRaw <- subset(resultsRaw, select = -15)

#Set data category
library(dplyr)
resultsRaw <- resultsRaw %>% mutate(Category = ifelse(Turbines == 'Al', 'Sep', ifelse(Vessels == 'NO', 'Ove', 'Sha')))
colnames(resultsRaw) <- c("Objective", "Duration", "Avail time", "Avail ener", "ProdLoss", "DirCosts", "VesCosts", "RepCosts", "TechCosts", "Turbines", "Dur", "Vessels", "Percent", "Size", "Category")

#Plot
library(ggplot2)
library(GGally)
resultsAgg <- aggregate(cbind(Objective, ProdLoss, DirCosts, VesCosts, RepCosts, TechCosts) ~ Category, resultsRaw, mean)
ggparcoord(resultsAgg, columns = c(2, 3, 4, 5, 6, 7), groupColumn = "Category", scale = "globalminmax") +
  geom_line(size = 1.5)

#Breakdown plots
library(crayon)
resultsDur <- aggregate(cbind(Objective, ProdLoss, DirCosts, VesCosts, RepCosts, TechCosts) ~ Category + Dur, resultsRaw, mean)
ggparcoord(resultsDur, columns = c(3, 4, 5, 6, 7, 8), groupColumn = "Category", scale = "globalminmax") +
  geom_line(aes(lty = as.factor(Dur)), size = 1.5) + 
  scale_linetype_manual(values=c("twodash", "dotted", "longdash", "solid", "blank", "dotdash", "dashed"))

resultsTurb <- aggregate(cbind(Objective, ProdLoss, DirCosts, VesCosts, RepCosts, TechCosts) ~ Category + Turbines, resultsRaw, mean)
ggparcoord(resultsTurb, columns = c(3, 4, 5, 6, 7, 8), groupColumn = "Category", scale = "globalminmax") +
  geom_line(aes(lty = as.factor(Turbines)), size = 1.5) + 
  scale_linetype_manual(values=c("twodash", "dotted", "longdash", "solid", "blank", "dotdash", "dashed"))

resultsVes <- aggregate(cbind(Objective, ProdLoss, DirCosts, VesCosts, RepCosts, TechCosts) ~ Category + Vessels, resultsRaw, mean)
ggparcoord(resultsVes, columns = c(4, 5, 6, 7, 8), groupColumn = "Category", scale = "globalminmax") +
  geom_line(aes(lty = as.factor(Vessels)), size = 1.5) + 
  scale_linetype_manual(values=c("twodash", "dotted", "longdash", "solid", "dotdash", "dashed"))

resultsPerc <- aggregate(cbind(Objective, ProdLoss, DirCosts, VesCosts, RepCosts, TechCosts) ~ Category + Percent, resultsRaw, mean) %>% transform(Percent = chr(Percent))
ggparcoord(resultsPerc, columns = c(4, 5, 6, 7, 8), groupColumn = "Category", scale = "globalminmax") +
  geom_line(aes(lty = as.factor(Percent)), size = 1.5) + 
  scale_linetype_manual(values=c("twodash", "dotted", "longdash", "solid", "dotdash", "dashed"))

resultsSize <- aggregate(cbind(Objective, ProdLoss, DirCosts, VesCosts, RepCosts, TechCosts) ~ Category + Size, resultsRaw, mean) %>% transform(Size = chr(Size))
ggparcoord(resultsSize, columns = c(4, 5, 6, 7, 8), groupColumn = "Category", scale = "globalminmax") +
  geom_line(aes(lty = as.factor(Size)), size = 1.5) + 
  scale_linetype_manual(values=c("twodash", "dotted", "longdash", "solid"))

#TODO: Check out parcoords ipv ggparcoord
#TODO: Transform Objective to Total (annual)
#TODO: Create "normalized" dataframe which normalizes all data by #turbines