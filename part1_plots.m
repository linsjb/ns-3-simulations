clc
clear

figure('NumberTitle', 'off', 'Name', 'Graphs');

rows = 1;
cols = 1;
subs = rows * cols;
n = 1:1:1000;

tiledlayout(rows, cols)

%file_names = ["lcg-m100-c1-a13", "ns3-m100-c1-a13"];
file_names = ["poirvn-m100-c1-a13"];
titles = ["Exponential distribution vs RVN simulation"]
legends = ["EXP", "RVN"];

for i = 1:subs
    nexttile()
    
    data = readtable("scratch/" + file_names(i) + ".csv");
    one = table2array(data(1,:))
    two = table2array(data(2,:));
    %three = table2array(data(3,:));

    p1 = plot(n, one, 'x')
    % p1.LineStyle = 'none';
    hold on
    p2 = plot(n, two, 'o')
    %p2.LineStyle = 'none';
    %hold on
    %p3 = plot(n, three, '-+')
    % p3.LineStyle = 'none';
    
    title(titles(i))
    xlabel("n")
    ylabel("Random number")
    legend(legends)
end