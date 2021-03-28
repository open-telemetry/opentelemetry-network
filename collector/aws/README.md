# AWS Collector

The `aws-collector` collects AWS information that cannot be otherwise collected by our flowmill
agent, and feeds that information to our pipeline server.

The collector is comprised of two components:
- the agent that periodically queries AWS and pushes the information to the pipeline server;
- a span within the pipeline server that consumes messages sent by `aws-collector` and enriches
  the flow.

# Running:
The `aws-collector` requires two pieces of settings to run: auth config and AWS credentials.

The TL;DR to build and run in a test environment within `benv` is this:
```bash
cd ~/out/
make -j aws-collector
export AWS_ACCESS_KEY_ID="your_access_key"
export AWS_SECRET_ACCESS_KEY="your_secret_access_key"
# make sure the server is running, as well as stunnel
src/collector/aws/aws-collector --auth-config=$HOME/src/misc/localhost_auth_config.yaml
```

## Auth Config
NOTE: this is being deprecated in favor of API keys.

Those are the same auth config required by the flowmill agent.

For a dev environment there is a [YAML file](https://github.com/Flowmill/flowmill/blob/master/misc/localhost_auth_config.yaml)
that can be used for testing purposes.

## AWS Credentials
Check the AWS SDK Developer Guide for [recommended ways to provide credentials](https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/credentials.html).

For testing, you can supply two environment variables:
```bash
export AWS_ACCESS_KEY_ID=your_access_key_id
export AWS_SECRET_ACCESS_KEY=your_secret_access_key
```
