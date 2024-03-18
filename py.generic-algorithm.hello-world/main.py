"""Hello world, with a genetic algorithm.

https://twitter.com/matthen2/status/1769368467067621791
"""

import random
import time
from dataclasses import dataclass
from itertools import chain
from typing import Iterable, List

_LETTERS = "abcdefghijklmnopqrstuvwxyz "


class Color:
    RED = "\033[91m"
    GREEN = "\033[92m"
    CYAN = "\033[96m"
    BOLD = "\033[1m"
    END = "\033[0m"


@dataclass
class Candidate:
    text: str
    fitness: int = -1
    in_focus: bool = False

    def display_str(self, target_str: str) -> str:
        prefix = "➤ " if self.in_focus else "  "
        if self.fitness < 0:
            return prefix + self.text
        out = prefix
        for char, target_char in zip(self.text, target_str):
            if char != target_char:
                out += Color.RED + char
            else:
                out += Color.GREEN + char
        return out + Color.END

    def set_fitness(self, target_str):
        self.fitness = sum(
            int(char == target_char) for char, target_char in zip(self.text, target_str)
        )


def reset_focus(population: List[Candidate]) -> None:
    for candidate in population:
        candidate.in_focus = False


def seed_population(
    population: List[Candidate], population_size: int, target_str_len: int
) -> Iterable[str]:
    """Seed a population with random candidates, and yields after every new one."""
    while len(population) < population_size:
        reset_focus(population)
        population.append(
            Candidate(
                text="".join(random.choice(_LETTERS) for _ in range(target_str_len)),
                in_focus=True,
            )
        )
        yield "Seeding the population"


def order_by_fitness(
    population: List[Candidate],
    target_str: str,
) -> Iterable[str]:

    # Show errors for all
    for candidate in population:
        if candidate.fitness >= 0:
            continue
        reset_focus(population)
        candidate.set_fitness(target_str)
        candidate.in_focus = True
        yield "Computing fitness"

    reset_focus(population)
    made_swap = True
    evens = True
    while made_swap:
        evens = not evens
        made_swap = False
        for i in range(int(evens), len(population), 2):
            if i + 1 >= len(population):
                continue
            candidate_a, candidate_b = population[i : i + 2]
            if candidate_a.fitness >= candidate_b.fitness:
                continue

            # Need to swap
            made_swap = True
            population[i] = candidate_b
            population[i + 1] = candidate_a

        yield "Sorting by fitness"


def remove_unfit(population: List[Candidate], num_fit_to_keep: int):
    while len(population) > num_fit_to_keep:
        population.pop()
        population[-1].in_focus = True
        yield "Removing unfit candidates"
    reset_focus(population)


def breed(parent_a: Candidate, parent_b: Candidate, mutation_prob: float) -> Candidate:
    text = ""
    for char_a, char_b in zip(parent_a.text, parent_b.text):
        if random.random() < mutation_prob:
            text += random.choice(_LETTERS)
        elif random.random() < 0.5:
            text += char_a
        else:
            text += char_b
    return Candidate(text=text)


def breed_new(population: List[Candidate], population_size: int, mutation_prob: float):
    num_fit = len(population)
    while len(population) < population_size:
        i = random.randint(0, num_fit - 1)
        j = (i + random.randint(1, num_fit - 1)) % num_fit

        parent_a = population[i]
        parent_b = population[j]
        reset_focus(population)
        parent_a.in_focus = True
        parent_b.in_focus = True
        child = breed(parent_a, parent_b, mutation_prob=mutation_prob)
        child.in_focus = True
        population.append(child)
        yield "Breeding new candidates"


def display(
    *,
    population: List[Candidate],
    label: str,
    population_size: int,
    num_columns: int,
    column_width: int,
    target_str: str,
) -> None:
    print("\n\n")
    print(
        Color.BOLD
        + Color.CYAN
        + label.center(column_width * num_columns)
        + Color.END
        + "\n"
    )
    num_rows = population_size // num_columns
    cells = [["" for _ in range(num_columns)] for _ in range(num_rows)]

    for i in range(population_size):
        row_idx = i % num_rows
        col_idx = i // num_rows

        if i >= len(population):
            cells[row_idx][col_idx] = " " * column_width
            continue

        padding = column_width - len(target_str) - 2
        cells[row_idx][col_idx] = population[i].display_str(target_str) + " " * padding

    for row in cells:
        print("   " + "".join(row))

    print("\n")


def main(
    target_str: str = "hello world",
    population_size: int = 48,
    num_fit_to_keep: int = 5,
    num_columns: int = 4,
    mutation_prob: float = 0.15,
):
    population: List[Candidate] = []
    column_width = len(target_str) + 6

    while True:
        for label in chain(
            seed_population(
                population,
                population_size=population_size,
                target_str_len=len(target_str),
            ),
            order_by_fitness(population, target_str=target_str),
            remove_unfit(population, num_fit_to_keep=num_fit_to_keep),
            breed_new(
                population, population_size=population_size, mutation_prob=mutation_prob
            ),
        ):
            time.sleep(0.016)
            print("\033[H\033[J", end="")
            display(
                population=population,
                label=label,
                population_size=population_size,
                num_columns=num_columns,
                column_width=column_width,
                target_str=target_str,
            )


if __name__ == "__main__":
    main()
