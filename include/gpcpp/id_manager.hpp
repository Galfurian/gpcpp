/// @file id_manager.hpp
/// @brief Keeps track of used IDs.

#pragma once

#include <unordered_set>

namespace gpcpp
{

/// @brief A class that manages unique IDs for different style types (e.g., textbox, line styles).
class id_manager_t
{
private:
    // A set to track used IDs for a particular style
    std::unordered_set<int> used_ids;

public:
    /// @brief Generates a unique ID that hasn't been used before.
    /// @return A unique integer ID.
    inline int generate_unique_id()
    {
        static int id = 1; // Initialize static ID counter.
        while (used_ids.find(id) != used_ids.end()) {
            ++id; // Increment until a unique ID is found.
        }
        this->add_id(id);
        return id;
    }

    /// @brief Checks if an ID has been used.
    /// @param id The ID to check.
    /// @return True if the ID is already used, false otherwise.
    inline bool is_used(int id) const { return used_ids.find(id) != used_ids.end(); }

    /// @brief Adds an ID to the manager's tracking set.
    /// @param id The ID to be added.
    /// @return True if added successfully, false if ID was already used.
    inline bool add_id(int id)
    {
        if (is_used(id)) {
            return false; // ID already used.
        }
        used_ids.insert(id);
        return true;
    }

    /// @brief Resets the used IDs.
    inline void clear() { used_ids.clear(); }
};

} // namespace gpcpp
