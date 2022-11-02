// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

inline ConnectorDirection reverse(ConnectorDirection direction)
{
  switch (direction) {
  case ConnectorDirection::UNKNOWN:
    return ConnectorDirection::UNKNOWN;
  case ConnectorDirection::REQUEST:
    return ConnectorDirection::RESPONSE;
  case ConnectorDirection::RESPONSE:
    return ConnectorDirection::REQUEST;
  default:
    assert(false); // invalid direction
    return ConnectorDirection::UNKNOWN;
  }
}

inline std::underlying_type_t<ConnectorDirection> to_int(ConnectorDirection direction)
{
  return static_cast<std::underlying_type_t<ConnectorDirection>>(direction);
}

inline std::underlying_type_t<FlowSide> operator+(FlowSide side)
{
  return static_cast<std::underlying_type_t<FlowSide>>(side);
}

inline FlowSide operator~(FlowSide side)
{
  return static_cast<FlowSide>(!static_cast<std::underlying_type_t<FlowSide>>(side));
}

inline FlowSide u8_to_side(u8 side)
{
  assert(side <= 1);
  return static_cast<FlowSide>(static_cast<std::underlying_type_t<FlowSide>>(side));
}

inline std::string_view to_string(FlowSide side, std::string_view fallback)
{
  switch (side) {
  case FlowSide::SIDE_A:
    return "SIDE_A";
  case FlowSide::SIDE_B:
    return "SIDE_B";
  default:
    return fallback;
  }
}

template <typename Out> Out &operator<<(Out &&out, FlowSide side)
{
  out << to_string(side);
  return out;
}
