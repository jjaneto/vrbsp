#! /usr/bin/env python3

import pandas as pd

if __name__ == "__main__":
    result = pd.read_csv("result_information.txt", sep=" ")
    result.loc["means"] = result.mean()
    print(result)
