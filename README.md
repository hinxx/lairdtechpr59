# Introduction

2016-12-06, Hinko Kocevar <hinko.kocevar@esss.se>


This is EPICS support for LairdTech PR-59 series temperature controllers.
See http://www.lairdtech.com/products/tc-xx-pr-59.


# Notes

* using asynDriver instead of streamdevice in order to handle bit masks and
to be able use commands $Ax (continuous logging)

* Steinhart coefficients (A, B, C) for temperature sensor are not calculated from R - T
points (TODO if needed). See https://en.wikipedia.org/wiki/Steinhart-Hart_equation.

* only temperature sensor 1 is supported (TODO for other sensors)

* no support for fans (TODO)

* no support for alarms (TODO)

* ON/OFF regulator mode not supported (TODO)

* not tested with load (TODO)
