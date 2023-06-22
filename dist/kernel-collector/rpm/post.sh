service=kernel-collector.service
if [ $1 -eq 1 ]; then
  # install
  systemctl preset $service >/dev/null 2>&1 || :
fi
