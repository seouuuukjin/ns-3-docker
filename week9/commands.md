# gnuplot
    plot "flow1.dat" using 1:2 title 'Flow1' with linespoints, "flow2.dat" using 1:2 title 'Flow2' with linespoints
# waf
    ./waf --run scratch/week9/week9 > week9-log.dat
end