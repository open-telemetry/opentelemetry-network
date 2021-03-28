# Runs the collector in local-only mode, for debugging

export FLOWMILL_INTAKE_HOST=localhost
export FLOWMILL_INTAKE_PORT=7001
export FLOWMILL_INTAKE_NAME=localhost
export FLOWMILL_AUTH_KEY_ID=KFIIXXXXXXXXXXXXXXXX
export FLOWMILL_AUTH_SECRET=KFIIXXXXXXXXXXXXXXXX

export local_cmd_args=(\
    --log-console 
    --authz-server=https://localhost:8001 
    --bpf-dump-file=/trace/bpf.raw
)

# set up certificate trust
cp minica.pem /etc/ssl/certs
c_rehash /etc/ssl/certs
mkdir -p /root/install/openssl/certs/
cp minica.pem /root/install/openssl/certs/
c_rehash /root/install/openssl/certs/

# make sure we have a /trace directory
mkdir -p /trace

# run mock authz and telemetry ports
/srv/mock_authz_server.py &
nc -k -l -p 7000 -s 127.0.0.1 > /trace/trace.raw &
stunnel stunnel-authz.conf
stunnel stunnel-telemetry.conf

