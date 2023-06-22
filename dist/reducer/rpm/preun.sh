service=reducer.service
if [ $1 -eq 0 ]; then
  # uninstall
  systemctl --no-reload disable $service > /dev/null 2>&1 || :
  systemctl stop $service > /dev/null 2>&1 || :
fi
