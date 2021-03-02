library(forecast)

data <- AirPassengers
seasonplot(data)
ann1 <- nnetar(data)
print(ann1)
accnfcst <- forecast(ann1,h=12)
autoplot(accnfcst)

sim <- ts(matrix(0,nrow=12L, ncol=9), start = end(data)[1]+1, frequency = 12)
for(i in seq(9))
{
  sim[,i] <-simulate(ann1, nsim=12)
}
autoplot(data) + autolayer(sim)