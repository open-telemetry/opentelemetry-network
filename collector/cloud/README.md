# Cloud Collector

The `cloud-collector` collects cloud information that cannot be otherwise collected by our flowmill
agent, and feeds that information to the reducer.

The collector is comprised of two components:
- the agent that periodically queries AWS and pushes the information to the pipeline server;
- a span within the pipeline server that consumes messages sent by `cloud-collector` and enriches
  the flow.

# Running:
The `cloud-collector` requires two pieces of settings to run: auth config and AWS credentials.

The TL;DR to build and run in a test environment within `benv` is this:
```bash
cd ~/out/
make -j cloud-collector
export AWS_ACCESS_KEY_ID="your_access_key"
export AWS_SECRET_ACCESS_KEY="your_secret_access_key"
# make sure the server is running, as well as stunnel
src/collector/cloud/cloud-collector --auth-config=$HOME/src/misc/localhost_auth_config.yaml
```

## AWS Credentials
Check the AWS SDK Developer Guide for [recommended ways to provide credentials](https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/credentials.html).

For testing, you can supply two environment variables:
```bash
export AWS_ACCESS_KEY_ID=your_access_key_id
export AWS_SECRET_ACCESS_KEY=your_secret_access_key
```
