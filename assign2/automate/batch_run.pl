#!/usr/bin/perl

use strict;
use warnings;

# Output file
 my $output_file = 'Output/Ghonim_Nordstrom_output3_(1)_mac_os_2.txt';
# my $output_file = 'Output/Ghonim_Nordstrom_output3_(2)_mac_os.txt';
# my $output_file = 'Output/Ghonim_Nordstrom_output3_(1)_linux.txt';
# my $output_file = 'Output/Ghonim_Nordstrom_output3_(1)_linux.txt';

# Open the file for writing
open(my $fh, '>', $output_file) or die "Could not open file '$output_file' $!";

# Print the header to the file with fixed widths for each column
printf $fh "%-20s %-15s %-15s\n", "Number of Threads", "Time (Seconds)", "Speedup";

# Variable to store time for single-thread execution
my $single_thread_time = 0;

# Run the program with different numbers of threads
for (my $num_threads = 1; $num_threads <= 16; $num_threads++) {
    # Execute the program and capture its output
    my $output = `./bin/TempGrid_HW3 $num_threads`;
    #my $output = `./bin/HW3_1 $num_threads`;

    # Extract the time from the output
    my ($time_in_seconds) = $output =~ /Time = \d+ nanoseconds\s+\(([\d\.]+) sec\)/;

    # Calculate speedup
    my $speedup = $num_threads == 1 ? 1 : $single_thread_time / $time_in_seconds;
    $single_thread_time = $time_in_seconds if $num_threads == 1;

    # Write the results to the output file with fixed widths for each column
    printf $fh "%-20d %-15f %-15f\n", $num_threads, $time_in_seconds, $speedup;
}

# Close the file
close $fh;

print "Results saved in $output_file\n";

