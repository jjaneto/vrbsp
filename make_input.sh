#!/usr/bin/env bash

instance=16
timeLimit=3600
formulation="W2"
begin=21
end=30

for ((i=begin; i<=end; i++)) do
  {
    echo ${instance}
    echo ${formulation}
    echo ${timeLimit}
    echo ${i}
  } > input;
  ./vrbsp input >> out${instance}_${formulation};
done
