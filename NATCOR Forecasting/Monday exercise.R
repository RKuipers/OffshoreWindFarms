load("C:/Users/Robin/OneDrive/Documenten/GitHub/OWFSim/NATCOR Forecasting/grocery.Rdata")
plot(y)

n <- length(y)
n

library(forecast)
library(smooth)
library(tsutils)

y.trn <-head(y,7*50)# That reads as take the first 50 weeks of the series.
y.tst <-tail(y,7*2)# ... and the last two weeks as test.

cma <-cmav(y.trn, outplot=TRUE)

seasplot(y.trn)
seasplot(y.trn,outplot=2)
seasplot(y.trn,outplot=3)
seasplot(head(y.trn,10*7))
seasplot(tail(y.trn,10*7))

dc <-decomp(y.trn, outplot=TRUE)
dc <-decomp(y.trn, outplot=TRUE, type="pure.seasonal")

#fit <-es(y.trn)
# Level model
#fit1 <-es(y.trn,model="ANN")
# Seasonal model
#fit2 <-es(y.trn,model="ANA")
# Linear trend model
#fit3 <-es(y.trn,model="AAN")
# Damped trend model
#fit4 <-es(y.trn,model="AAdN")
# Trend seasonal model
#fit5 <-es(y.trn,model="AAA")
# Damped trend seasonal model
#fit6 <-es(y.trn,model="AAdA")

#aicc <-c(fit1$ICs[2],fit2$ICs[2],fit3$ICs[2],fit4$ICs[2],fit5$ICs[2],fit6$ICs[2])
# We name the aicc vectr to easily identify which is which
#names(aicc) <-c("ANN","ANA","AAN","AAdN","AAA","AAdA")
#aicc

y.ins <-head(y.trn,48*7)
y.val <-tail(y.trn,2*7)
h <- 7

# Note that now I will be using y.ins, instead of y.trn
fit1v <-es(y.ins,model="ANN")
fit2v <-es(y.ins,model="ANA")
fit3v <-es(y.ins,model="AAN")
fit4v <-es(y.ins,model="AAdN")
fit5v <-es(y.ins,model="AAA")
fit6v <-es(y.ins,model="AAdA")
fit7v <-es(y.ins,model="MNA")

frc1v <-forecast(fit1v,h=h)
frc2v <-forecast(fit2v,h=h)
frc3v <-forecast(fit3v,h=h)
frc4v <-forecast(fit4v,h=h)
frc5v <-forecast(fit5v,h=h)
frc6v <-forecast(fit6v,h=h)
frc7v <-forecast(fit7v,h=h)

# And the naive:
frc8v <-tail(y.ins,frequency(y.ins))[1:h]# that copies the last season
# using the function frequency() to find how long is a season and
# copy the last h of those observations.

err1v <-mean(abs(y.val[1:h]-frc1v$mean))
# y.val[1:h] gives us the first 7 observations of y.val
# frc1v$mean gives us the point forecasts of frc1v
# mean(abs( )) gives us the MAE.
err2v <-mean(abs(y.val[1:h]-frc2v$mean))
err3v <-mean(abs(y.val[1:h]-frc3v$mean))
err4v <-mean(abs(y.val[1:h]-frc4v$mean))
err5v <-mean(abs(y.val[1:h]-frc5v$mean))
err6v <-mean(abs(y.val[1:h]-frc6v$mean))
err7v <-mean(abs(y.val[1:h]-frc7v$mean))
# For the naive we just have the numeric values in frc8v, so we do
# not need the suffix $mean
err8v <-mean(abs(y.val[1:h]-frc8v))

errv <-c(err1v, err2v, err3v, err4v, err5v, err6v, err7v, err8v)
names(errv) <-c("ANN","ANA","AAN","AAdN","AAA","AAdA","MNA","Naive")

omax <-length(y.val)-h+1

# This is what we will be running
models <-c("ANN", "ANA", "AAN", "AAN", "AAA", "AAA", "MNA",  "Naive")
damped <-c(FALSE, FALSE, FALSE, TRUE,  FALSE, TRUE,   FALSE, FALSE)
# And this is where we will store things
# Forecast errors across forecast origins
err <-array(NA,c(omax,8))# This has omax rows and 8 columns,
# one for each different forecasting method.
# Forecasts for a given origin
frcs <-array(NA,c(h,8))

# For each forecast origin
for (o in 1:omax){
  
  # Split training set
  y.ins <- head(y.trn,48*7-1+o) # As o increases, so will the in-sample.
  y.val <- tail(y.trn,2*7-o+1) # As o increases, the validation will decrease.
  
  # Fit and forecast will all exponential smoothing models
  for (m in 1:7){
    
    fitTemp <- ets(y.ins,model=models[m],damped=damped[m])
    frcs[,m] <- forecast(fitTemp,h=h)$mean
    err[o,m] <- mean(abs(y.val[1:h] - frcs[,m]))
  }
  
  # Forecast using the seasonal naive
  frcs[,8] <- tail(y.ins,frequency(y.ins))[1:h]
  err[o,8] <- mean(abs(y.val[1:h] - frcs[,8]))
}
colnames(err) <-c("ANN", "ANA", "AAN", "AAdN", "AAA", "AAdA", "MNA",  "Naive")
errMean <-colMeans(err)

boxplot(err)
nemenyi(err,plottype="matrix")