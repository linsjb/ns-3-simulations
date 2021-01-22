clc
clear

file = "p2p_queue_gs.txt";
data = readtable("scratch/" + file);
numbers = table2array(data(:,2));

avg = mean(numbers)
queueSum = sum(numbers)

mu = 10000;
lambda = 2500;
n = 1/3;

Tp = n/mu + 1/mu

% Transmission time
Taverage = Tp / lambda

Tmean = Taverage / queueSum
