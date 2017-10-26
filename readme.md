A simple downloader project written in c by Negar Mirgati
To kill processes listening on a particular port enter :
lsof -n -i4TCP:[PORT] | grep LISTEN | awk '{ print $2 }' | xargs kill
