./master/master -a -n > output &
sleep 1
./sample/random > output &
sleep 1
./sample/sweep > output &

