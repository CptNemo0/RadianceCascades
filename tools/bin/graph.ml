module StringMap = Map.Make (String)

let extract_lines (filename : string) =
  In_channel.with_open_text filename (fun input_channel ->
      let rec loop acc =
        match In_channel.input_line input_channel with
        | Some line ->
            let raw_parts = String.split_on_char ' ' line in
            let parts = List.filter (fun s -> s <> "") raw_parts in

            let new_acc =
              match parts with
              | label :: samples_str ->
                  let samples_int =
                    List.filter_map
                      (fun s ->
                        match int_of_string_opt s with
                        | Some i when i >= 0 -> Some i
                        | _ -> None)
                      samples_str
                  in
                  StringMap.add label samples_int acc
              | [] -> acc
            in
            loop new_acc
        | None -> acc
      in
      loop StringMap.empty)

(* Filters a single list of integers, removing anything beyond 3 standard deviations *)
let filter_outliers (samples : int list) =
  let n = float_of_int (List.length samples) in

  (* If there is 1 or 0 elements, we can't calculate a meaningful variance *)
  if n <= 1.0 then samples
  else
    (* 1. Calculate the Mean *)
    let sum = List.fold_left (fun acc x -> acc +. float_of_int x) 0.0 samples in
    let mean = sum /. n in

    (* 2. Calculate the Standard Deviation *)
    let sq_diff_sum =
      List.fold_left
        (fun acc x ->
          let diff = float_of_int x -. mean in
          acc +. (diff *. diff)
          (* Squaring the difference *))
        0.0 samples
    in
    let sd = sqrt (sq_diff_sum /. n) in

    (* 3. Filter the list *)
    let threshold = 3.0 *. sd in
    List.filter
      (fun x ->
        let distance_from_mean = abs_float (float_of_int x -. mean) in
        distance_from_mean <= threshold)
      samples

(* Applies the filter to every list inside the StringMap *)
let process_data (data_map : int list StringMap.t) =
  StringMap.map filter_outliers data_map

let plot_with_gnuplot (data_map : int list StringMap.t) =
  let gp_out = Unix.open_process_out "gnuplot -persist" in

  Printf.fprintf gp_out "set title 'Samples by Label'\n";
  Printf.fprintf gp_out "set xlabel 'Sample Index'\n";
  Printf.fprintf gp_out "set ylabel 'Value'\n";
  Printf.fprintf gp_out "set key outside right top\n";
  Printf.fprintf gp_out "set key spacing 1.5\n";

  let labels = StringMap.bindings data_map |> List.map fst in
  let plot_cmd =
    labels
    |> List.map (fun lbl -> Printf.sprintf "'-' with lines title '%s'" lbl)
    |> String.concat ", "
  in

  Printf.fprintf gp_out "plot %s\n" plot_cmd;

  StringMap.iter
    (fun _ samples ->
      List.iteri
        (fun i value -> Printf.fprintf gp_out "%d %d\n" i value)
        samples;
      Printf.fprintf gp_out "e\n")
    data_map;
  flush gp_out;
  ignore (Unix.close_process_out gp_out)

let read_lines_from_cmd () =
  if Array.length Sys.argv <> 2 then print_endline "Usage: graph <filename>"
  else
    try
      let raw_data_map = extract_lines Sys.argv.(1) in
      let clean_data_map = process_data raw_data_map in
      Printf.printf "Successfully parsed %d labels. Generating graph...\n"
        (StringMap.cardinal clean_data_map);
      plot_with_gnuplot clean_data_map
    with Sys_error msg -> Printf.printf "Error: %s\n" msg

let () = read_lines_from_cmd ()
