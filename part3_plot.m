clc
clear

figure('NumberTitle', 'off', 'Name', 'Graphs');

file_names = ["p2p_queue_gs", "csma_queue_gs"];
legends = ["P2P", "CSMA"];
    
data1 = readtable("scratch/" + file_names(1) + ".txt");
data2 = readtable("scratch/" + file_names(2) + ".txt");

plot(table2array(data1(:,1)), table2array(data1(:,2)), 'x')
hold on
plot(table2array(data2(:,1)), table2array(data2(:,2)), 'O')


xlabel("Time")
ylabel("Queue size")
legend(legends)
