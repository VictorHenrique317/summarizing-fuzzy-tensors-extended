DATE=$(date +'%d_%m_%Y'); screen -r pts-0.gorgona5 -X logfile screen_logs/$DATE.txt; screen -r pts-0.gorgona5 -X log
