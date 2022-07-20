#pragma once

#include "utils.hpp"

#include <rpp/fwd.hpp>

rpp::dynamic_observable<sf::RectangleShape> get_shapes_to_draw(const rpp::dynamic_observable<CustomEvent>& events);