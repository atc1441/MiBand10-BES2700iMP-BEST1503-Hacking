#!/usr/bin/env perl

use strict;
use File::Copy;

my $pgm_name = $0;
$pgm_name =~ s%.*/%%;
if (scalar(@ARGV) != 3) {
    print "Usage:\n\t$pgm_name <elf_file> <org_bin_file> <bin_file>\n";
    exit 1;
}

my $elf_file = $ARGV[0];
my $org_bin_file = $ARGV[1];
my $out_file = $ARGV[2];
my $bin_file = $out_file . "_new";

my $base_pattern = "<##BASE##>";

my $file_size;

my $err = 0;
my $read_size = 0;
my $result;
my $data;
my $build_info_addr;
my $load_addr;
my $build_info_offset;
my $build_info_str;
my $build_info_len;
my @lines;
my $item;
my %symbols;
my @nm_result;
my $key;
my $new_str;
my $new_len;
my $sym;
my $i;

copy($org_bin_file, $bin_file) || die "Copy file failed: $!";

if (!open(FH, "+<", $bin_file)) {
    die "*** Failed to open binary file: $bin_file : $!\n\t";
}
if (!binmode(FH)) {
    close(FH);
    die "Failed to change to binary mode: $bin_file : $!\n\t";
}
$file_size = (stat(FH))[7];

if (!seek(FH, 12, 0)) {
    close(FH);
    die "Failed to seek: $bin_file : $!\n\t";
}
$result = read FH, $data, 4;
if (!defined($result)) {
    close(FH);
    die "*** Failed to read binary file: $bin_file : $!\n";
}
$build_info_addr = unpack("L", $data);

if (!seek(FH, -4, 2)) {
    close(FH);
    die "Failed to seek: $bin_file : $!\n\t";
}
$result = read FH, $data, 4;
if (!defined($result)) {
    close(FH);
    die "*** Failed to read binary file: $bin_file : $!\n";
}
$load_addr = unpack("L", $data);

$build_info_offset = $build_info_addr - $load_addr;

#printf "build_info_addr=0x%08X load_addr=0x%08X build_info_offset=%u\n", $build_info_addr, $load_addr, $build_info_offset;

if ($build_info_addr <= $load_addr) {
    printf "*** Bad build_info_addr=0x%08X (should > load_addr=0x%08X)\n", $build_info_addr, $load_addr;
    close(FH);
    exit(1);
}

if ($build_info_offset <= 16 || $build_info_offset >= $file_size) {
    printf "*** Bad build_info_offset=%u (should > 16 && < file_size=%u)\n", $build_info_offset, $file_size;
    close(FH);
    exit(1);
}

if (!seek(FH, $build_info_offset, 0)) {
    close(FH);
    die "Failed to seek: $bin_file : $!\n\t";
}
$result = read FH, $data, ($file_size - $build_info_offset);
if (!defined($result)) {
    close(FH);
    die "*** Failed to read binary file: $bin_file : $!\n";
}
$build_info_str = unpack("Z*", $data);
$build_info_len = length($build_info_str);

#printf "build_info_str={%u}[%s]\n", $build_info_len, $build_info_str;

@lines = split(/\r|\n/, $build_info_str);

for my $item (@lines) {
    if ($item =~ /(\w+)=$base_pattern/) {
        $symbols{$1} = undef();
        #printf "Found: $1\n";
    }
}

if (scalar(keys(%symbols)) == 0) {
    #printf "No symbol to be filled. Quit.\n";
    goto _exit;
}

@nm_result = qx/arm-none-eabi-nm $elf_file/;

for $item (@nm_result) {
    for $key (keys(%symbols)) {
        if ($item =~ /^([0-9a-fA-F]{1,8})\s+.*\s+$key$/) {
            $symbols{$key} = "0x" . uc($1);
            #printf "Got: $key=$symbols{$key}\n";
        }
    }
}

$new_str = $build_info_str;
for $key (keys(%symbols)) {
    if (!defined($symbols{$key})) {
        printf "*** Warning: Failed to get symbol address: $key\n";
        next;
    }
    $sym = $key . "=";
    $i = index($new_str, $sym);
    if ($i < 0) {
        printf "*** Warning: Failed to locate symbol in build_info: $key\n";
        next;
    }
    $i += length($sym);
    substr($new_str, $i, 10, $symbols{$key});
}

$new_len = length($new_str);
#printf "new_str={%u}[%s]\n", $new_len, $new_str;

if (!seek(FH, $build_info_offset, 0)) {
    close(FH);
    die "Failed to seek: $bin_file : $!\n\t";
}
$result = print FH $new_str;
if (!$result) {
    close(FH);
    die "*** Failed to write binary file: $bin_file : $!\n";
}

_exit: {;}

close(FH);

move($bin_file, $out_file) || die "Move file failed: $!";

exit 0;
