# Azure Coast Heist Planner

A C++ optimization challenge set against the glittering, high-stakes world of the French Riviera. Choose which vaults to open, find a valid way to unlock each one, and make the most of a limited time window.

## The Story

The moon hung over Monaco like a diamond in a velvet sky. I was crouched beside a safe hidden in a turret of Princess Delacroix's suite at the Hôtel de Paris. The casino hummed below; champagne was clearly flowing. A man in a hat appeared in the distance. I recognized him at once: Inspector Trachta. Again. How does he always show up just as I turn the final dial?

"I've got you this time, Le Chat!" he shouted as the lights came on. But I was already at the edge of the roof. "Prove it, Inspector," I said, and jumped. All I left behind was a shadow and the scent of cologne.

Three nights later, in Nice, the jewelry museum was exhibiting the famous Cœur de Lune diamond. Trachta guarded the entrance himself, nervously pacing in dress shoes that looked suspiciously borrowed from the embassy. "Tonight, gentlemen," he told his team, "the cat goes in the bag."

At midnight, the alarm rang. The cameras caught only a gloved hand passing over the glass case. Two minutes later, the case was empty. On the floor lay a calling card bearing the drawing of a cat in a top hat.

Trachta ran toward the harbor, where a boat was pulling away. "Surrender!" he shouted. The only reply was a distant piano chord. Someone in a nearby villa was playing *La Vie en Rose*.

By morning, the newspapers were writing about the "Phantom of the French Riviera." Trachta sat over a croissant in a café, his pipe burning low. On the table was a napkin with a single line: "Au revoir, Inspector. See you in Paris."

## The Challenge

Implement `plan_heist` to select vaults that maximize the total value collected without exceeding the time limit. For each selected vault, return its input index and a configuration describing which pebbles to move to unlock it.

### Vaults and Locks

Each vault is described by a `Vault` value:

| Field | Description |
| --- | --- |
| `value` | Non-negative value of the contents |
| `pebbles` | Number of pebbles in the lock |
| `missing_connections` | Pairs of pebbles that are not connected in the lock |

Initially, every pebble is on the left side of the vault door. To unlock a vault, move exactly half of its pebbles to the right, such that every pebble on the right is connected to every pebble on the left. The unlock time is the number of pebbles moved. Some vaults may be impossible to unlock.

The `missing_connections` list is sparse: it contains the connections that are absent, rather than those that are present.

### Time Budget

The function also receives:

| Argument | Description |
| --- | --- |
| `transition_time` | Non-negative time required to move between any two vaults |
| `max_time` | Maximum time available in the vault room; this value is relatively small |

The time spent taking the contents of a vault is negligible. All calculations are expected to fit within the range of the `int` type.

## Interface

```cpp
std::vector<UnlockingSequence> plan_heist(
    const std::vector<Vault>& vaults,
    unsigned transition_time,
    unsigned max_time
);
```

An `UnlockingSequence` contains a vault's zero-based input index (`vault_id`) and a `moved_pebbles` vector. A `true` entry means that the corresponding pebble must be moved to the right side.