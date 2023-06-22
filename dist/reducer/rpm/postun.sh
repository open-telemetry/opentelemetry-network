service=reducer.service
systemctl daemon-reload >/dev/null 2>&1 || :
if [ $1 -ge 1 ]; then
  # upgrade
  systemctl try-restart $service >/dev/null 2>&1 || :
fi
