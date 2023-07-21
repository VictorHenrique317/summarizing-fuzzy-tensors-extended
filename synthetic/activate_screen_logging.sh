log_filename="screen_logs/$(date +'%d_%m_%Y').txt"
session_name=$(screen -ls | grep -oP '(?<=\d\.)\S+')

screen -r $session_name -X logfile screen_logs/$filename
screen -r $session_name -X log
