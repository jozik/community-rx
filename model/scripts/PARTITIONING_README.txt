How to partition the graph

Run pn2metis.py to create a metis format network file. This will create a
place_network.graph file and a numeric_id_idx.csv in the same directory as the
places input.

Run Metis, passing it the graph file the number of processes to partition over.
For example,

  ~/metis-5.1.0/build/Darwin-x86_64/programs/gpmetis place_network.graph 128

That generates a place_network.graph.part.128 file where the 128 is the
number of processes.

To set the rank of the places and persons using the Metis partition graph,
run assign_ranks_from_metis_output.py passing it the path to the persons file,
places file, and process count. For example,

python assign_ranks_from_metis_output.py  \
~/Documents/crx_input/southside/southside_zip16_crx_persons_v3.csv  \
~/Documents/crx_input/southside/southside_zip16_crx_places_v3.csv \
128
