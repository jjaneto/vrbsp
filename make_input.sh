#!/usr/bin/env bash

instance=8
timeLimit=3600
formulation="W2"
begin=1
end=1
area="D10000x10000"

for ((i=begin; i<=end; i++)) do
  {
    echo ${instance}
    echo ${area}
    echo ${formulation}
    echo ${timeLimit}
    echo ${i}
  } > input;
  ./vrbsp input >> out${instance}_${formulation};
done
