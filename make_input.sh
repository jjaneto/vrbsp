#!/usr/bin/env bash

instance=8
timeLimit=3600
formulation="W2"
begin=1
end=1
area="D1000x1000"

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
