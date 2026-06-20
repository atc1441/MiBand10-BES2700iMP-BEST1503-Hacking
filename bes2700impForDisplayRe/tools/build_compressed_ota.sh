#/bin/bash
i=0
size=0
isize=`stat -c %s $1`
echo "input file:$1,isize:$isize --> out file:$2"
split -a3 -d -b 131072 $1
echo -e -n "\xff\xff\xff\xff" > magic.bin
echo "CRC32_OF_IMAGE=0x00000000" > crc.bin
cp magic.bin $2
for file in `ls x0??`;
do
    echo "compressing $file"
    lzma -z -k -f -9 $file
    isize=`stat -c %s $file.lzma`
    echo "$file size is $isize"
    echo $isize | tr -d '\n' | xargs -i printf "%08x" {} | xargs echo -i {} | xxd -r -ps > tmp.bin
    cat tmp.bin $file.lzma >> $2
done
cat crc.bin >> $2
osize=`stat -c %s $2`
echo "osize=$osize"
rm tmp*.bin
rm *.lzma
rm x0??
