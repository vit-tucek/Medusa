#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

output_dir="${1:-output}"

for bin in mate_interact drawpullback draw_pb_sphere; do
  if [[ ! -x "./$bin" ]]; then
    make all
    break
  fi
done

mkdir -p "$output_dir"

run_case() {
  local p1="$1"
  local q1="$2"
  local p2="$3"
  local q2="$4"
  local iters="$5"
  local tag="$6"

  local param_file="${output_dir}/${tag}.txt"
  local plane_file="${output_dir}/${tag}p.eps"
  local sphere_file="${output_dir}/${tag}s.eps"

  printf '%s\n' \
    "$p1" "$q1" "$p2" "$q2" "$param_file" "$iters" "q" \
    | ./mate_interact > "${output_dir}/${tag}_mate.log"

  printf '%s\n' \
    "$plane_file" "$param_file" "-3" "3" "-3" "3" "120" \
    | ./drawpullback > "${output_dir}/${tag}_plane.log"

  printf '%s\n' \
    "$sphere_file" "$param_file" "120" \
    | ./draw_pb_sphere > "${output_dir}/${tag}_sphere.log"

  echo "Created $plane_file and $sphere_file"
}

# Two different matings so the output is not just a single trivial example.
run_case 1 7 3 7 35 m17v37
run_case 1 5 2 7 35 m15v27

echo "Done. Graphics are in ./${output_dir}"
