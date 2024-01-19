# TuringV2

Second of His name. Turing Gardner with the second version of the development board.

# January 19, 2038, The doomsday date!

This implemetation relies heavily on the timeLib library and use its now(); to time execution of non time-senstive tasks such as turn on and off the pump and send new data for the available services. When the unix timesatamp overflow, the overall work flow of the code should not change much, there will be a turning on of the pump and a sending of the data to the front end servers at 03:14:07 19/01/2038 but only once, after that everything should be normal. The things that may break are syncing time online and obviously the readable prints of the dates will be inacurrate.
the timestamps will also be afected but if a log is needed one could always just add INT_MAX to the timestamp and calculate the time from there.

## 
