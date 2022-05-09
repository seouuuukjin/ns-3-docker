# gnuplot
    plot "week11-flow1.dat" using 1:2 title 'Flow1' with linespoints, "week11-flow2.dat" using 1:2 title 'Flow2' with linespoints, "week11-background-flow.dat" using 1:2 title 'Background Flow' with linespoints
# waf
    ./waf --run scratch/week11/week11 > week11hw-log.dat
# awk
    awk '$1=="0" {print $2 "\t" $3}' week11hw-log.dat > week11-background-flow.dat
    awk '$1=="1" {print $2 "\t" $3}' week11hw-log.dat > week11-flow1.dat
    awk '$1=="2" {print $2 "\t" $3}' week11hw-log.dat > week11-flow2.dat
end