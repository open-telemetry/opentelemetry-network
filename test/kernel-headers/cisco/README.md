- run numbered scripts in order
- verify that the agent's binary is called with the expected kernel headers source, e.g.:

    exec /srv/flowmill-agent --host-distro debian --kernel-headers-source fetched --log-console --debug

- verify that the agent connects to the server and that it reaches the "Telemetry is flowing" message,
  at which point you can hit Ctrl-C and move to the next numbered script
