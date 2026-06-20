#!/usr/bin/env perl

use strict;

my $pgm_name = $0;
$pgm_name =~ s%.*/%%;
if (scalar(@ARGV) < 2) {
    print "Usage:\n\t$pgm_name <input_file> <output_file> [ <bytes_per_line> ] [ -hex | -bin | -oct ]\n";
    print "\t\t\t[ -hamm ] [ -crc ] [ -flash ] [ -little-endian | -big-endian ]\n";
    exit 1;
}

#use constant MAGIC_NUMBER => (0x45, 0x00, 0xef, 0xd9);
use constant MAGIC_NUMBER => (0x1C, 0xEC, 0x57, 0xBE);
my $magic_num_index = 0;

my $input_file = $ARGV[0];
my $output_file = $ARGV[1];
my $bytes_per_line = 4;
my $output_format = "%02x";
my $binary_flag = 0;
my $hamm_flag = 0;
my $crc_flag = 0;
my $flash_flag = 0;
my $big_endian_flag = 0;

my $crc_value = 0;

my $argCnt = 2;
my $argValue;
while (defined($ARGV[$argCnt])) {
    $argValue = $ARGV[$argCnt];
    $argCnt++;

    if ($argValue eq "-oct") {
        $output_format = "%03o";
        $binary_flag = 0;
    }
    elsif ($argValue eq "-bin") {
        $output_format = "%08b";
        $binary_flag = 1;
    }
    elsif ($argValue eq "-hex") {
        $output_format = "%02x";
        $binary_flag = 0;
    }
    elsif ($argValue eq "-hamm") {
        $hamm_flag = 1;
        $crc_flag = 1;
    }
    elsif ($argValue eq "-crc") {
        $crc_flag = 1;
    }
    elsif ($argValue eq "-flash") {
        $flash_flag = 1;
    }
    elsif ($argValue eq "-little-endian") {
        $big_endian_flag = 0;
    }
    elsif ($argValue eq "-big-endian") {
        $big_endian_flag = 1;
    }
    elsif ($argValue =~ /^(\d+)$/) {
        $bytes_per_line = $1;
    }
    else {
        print "Unknown option: $argValue\n";
        exit 2;
    }
}

if ($hamm_flag) {
    if ($bytes_per_line != 4) {
        print "Hamming encoding requires 4 bytes per line in output file\n";
        exit 4;
    }
    if (!$binary_flag) {
        print "Hamming encoding requires binary output format\n";
        exit 5;
    }
    if (!$big_endian_flag) {
        print "Hamming encoding requires little-endian format\n";
        exit 5;
    }
    $hamm_flag = 1;
}

if ($crc_flag) {
    if ($bytes_per_line != 4) {
        print "CRC calculation requires 4 bytes per line in output file\n";
        exit 6;
    }
}

my $err = 0;
my $input_file_size = 0;
my $read_size = 0;
my $seq_no = 0;
my @byte;
my $result;
my $data;
my $i;
my $outputByte;

open(FH, "<", $input_file) || die "Failed to open input file: $input_file : $!\n\t";
if (!binmode(FH)) {
    close(FH);
    die "Failed to change to binary mode: $input_file : $!\n\t";
}
$input_file_size = (stat(FH))[7];

if (!open(OUTPUT, ">", $output_file)) {
    close(FH);
    die "*** Failed to open output file: $output_file : $!\n\t";
}

while ($err == 0) {
    $result = read FH, $data, $bytes_per_line;
    if (!defined($result)) {
        $err = 1;
        print "*** Failed to read input file: $input_file : $!\n";
        last;
    }
    elsif ($result == 0) {
        # end of file
        last;
    }
my $no_use = q`
    elsif ($result != $bytes_per_line) {
        $err = 1;
        print "*** Unexpected alignment in input file: $input_file\n";
        last;
    }
`;
    $read_size += $result;

    @byte = unpack("C$bytes_per_line", $data);

    if ($flash_flag && $magic_num_index < scalar(MAGIC_NUMBER)) {
        # Write magic number at flash start address
        for ($i=0; $i<$bytes_per_line; $i++) {
            if ($byte[$i] == 0xff)
            {
                $byte[$i] = (MAGIC_NUMBER)[$magic_num_index];
                $magic_num_index++;
            }
            else
            {
                $magic_num_index = scalar(MAGIC_NUMBER);
            }

            if ($magic_num_index >= scalar(MAGIC_NUMBER))
            {
                last;
            }
        }
    }

    if ($crc_flag == 1) {
        my $word = $byte[0] | ($byte[1]<<8) | ($byte[2]<<16) | ($byte[3]<<24);
        $word = 0 if ($read_size == $input_file_size);
        crc_calc($word);
        if ($read_size == $input_file_size) {
            $byte[0] = $crc_value & 0xFF;
            $byte[1] = ($crc_value >> 8) & 0xFF;
            $byte[2] = ($crc_value >> 16) & 0xFF;
            $byte[3] = ($crc_value >> 24) & 0xFF;
        }
    }

    for ($i=0; $i<$bytes_per_line; $i++) {
        if ($big_endian_flag)
        {
            $outputByte = $byte[$i];
        }
        else
        {
            $outputByte = $byte[$bytes_per_line-1-$i];
        }
        $outputByte = 0 if (!defined($outputByte));

        printf OUTPUT $output_format, $outputByte;
    }

    if ($hamm_flag == 1) {
        printf OUTPUT "%06b", hamm6_enc(($byte[3]<<24)|($byte[2]<<16)|($byte[1]<<8)|$byte[0]);
    }
    printf OUTPUT "\n";

    $seq_no++;
}

close(OUTPUT);
close(FH);

$err;


sub hamm6_enc($)
{
    my $data = shift;
    my $parity = 0;
    my $feedback;
    my $i;

    for ($i=0; $i<32; $i++) {
        $feedback = (($data >> 31) ^ ($parity >> 5)) & 0x1;
        $parity = ((($parity << 1) & ~0x2) | ((($parity << 1) & 0x2) ^ ($feedback << 1)) | $feedback) & 0x3F;
        $data = ($data << 1) & 0xFFFFFFFF;
    }

    $parity;
}

sub crc_calc($)
{
    my $data = shift;
    my $new_msb = ($crc_value ^ ($crc_value >> 4) ^ ($crc_value >> 8) ^
                    ($crc_value >> 12) ^ ($crc_value >> 17) ^
                    ($crc_value >> 20) ^ ($crc_value >> 23) ^
                    ($crc_value >> 28)) & 0x1;
    $crc_value = (($new_msb << 31) | ($crc_value >> 1)) ^ $data;
}

