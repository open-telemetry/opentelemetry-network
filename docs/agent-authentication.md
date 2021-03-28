---
description: How the agent (collector) authentication works.
---

# Agent authentication

To be allowed to send telemetry to the pipeline server, agents \(collectors\) need to present a valid and unexpired authentication token obtained from the _authz service_.

The following schematic shows the parts involved in this process.

![Parts Interaction Schema](./assets/authz.svg)

## In a nutshell

1. Agent connects to the authz service and call the _GetTokenFromKey_ RPC operation, supplying a pre-configured key ID and secret. 
2. The authz service looks for an entry with the matching key ID/secret pair in the tenant database. If one is found, the service constructs a token \(JWT\) containing the key ID, key type and tenant ID.
3. If successful, the _GetTokenFromKey_ operation returns the token signed with the authz service's private key.
4. Agent sends the _authz\_authenticate_ message to the server, providing the signed token previously obtained from the authz service.
5. The server decrypts the token using the authz service's public key and then validates its content. Validation involves checking the token type \(must be an agent token, not API token\), tenant ID \(must match the allowed tenant ID supplied with the _--allowed-tenant-id_ command-line parameter\), and expiration time.

## Agent

The agent needs to be configured with the URL of the authz service. This is done either through the _FLOWMILL\_AUTHZ\_SERVER_ environment variable or the _--authz-server_ command-line parameter. The command-line parameter trumps the environment variable.

To get a token, the agent will call authz service's _GetTokenFromKey_ operation. This is usually done through authz service's HTTP gateway, meaning a HTTP GET is made. The request URL is _&lt;service-url&gt;/api/v1/auth/keys/&lt;key\_id&gt;_. The key secret is passed in the _Authorization_ header.

Curl example:

`$ curl https://app.flowmill.com/api/v1/auth/keys/<key_id> -H "Authorization: Bearer <secret>"`

## Server

The set of messages that the server accepts from agents is specified in the _flowmill.render_ file. Some of these messages are specified with the _no\_authorization\_needed_ keyword. Those messages are accepted before the connected agent authenticates itself. One of those messages is the _authz\_authenticate_ message.

The _authz\_authenticate_ message contains the signed token that an agent obtains from the authz service. The token is signed with authz service's private key. The server will validate the token using authz service's public key, which is preconfigured with the _--authz-public-key_ command-line parameter and defaults to _/etc/flowmill/authz-token-public.pem_.

When a running a server as a K8s container, the _/etc/flowmill/authz-token-public.pem_ path is a link to _/config/public/authz/public.pem_, which, in turn, is linked to the chart's _jwt.publicKey_ value.

## Authz service

The _\(key\_id, secret\)_ pair that an agent provides in the _GetTokenFromKey_ request are used to look up a matching row in the _api\_keys_ table. The row contains _key\_type_ and _tenant\_map\_id_ values. The _tenant\_map\_id_ references a row in the _tenant\_map_ table, from which _tenant\_id_ value is obtained. The _\(key\_id, key\_type, tenant\_id\)_ n-tuple is then used to construct a resulting token.

