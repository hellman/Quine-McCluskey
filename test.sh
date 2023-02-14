#!/bin/bash -e

for f in tests_ext/*.in; do
	echo $f
	echo
	echo "#" SparseQMCext:
	./sparseqmcext "$f" /tmp/out_sparseqmcext
	echo
	filename=$(basename $f)
	fout=$(dirname $f)/${filename%.*}.out
	if [[ -f "$fout" ]]; then
		diff /tmp/out_sparseqmcext "$fout"
	else
		cp /tmp/out_sparseqmcext "$fout"
	fi
	rm -f /tmp/out_sparseqmcext
	echo
	echo
done

for f in tests2/*.in tests/*.in; do
	echo $f
	echo
	echo "#" DenseQMC:
	./denseqmc "$f" /tmp/out_denseqmc
	echo
	echo "#" SparseQMC:
	./sparseqmc "$f" /tmp/out_sparseqmc
	echo
	echo "#" SparseQMCext:
	./sparseqmc "$f" /tmp/out_sparseqmcext
	echo
	echo "#" Diff:
	sha256sum /tmp/out_denseqmc /tmp/out_sparseqmc /tmp/out_sparseqmcext
	diff /tmp/out_denseqmc /tmp/out_sparseqmc || exit -1
	diff /tmp/out_denseqmc /tmp/out_sparseqmcext || exit -1
	filename=$(basename $f)
	fout=$(dirname $f)/${filename%.*}.out
	if [[ -f "$fout" ]]; then
		diff /tmp/out_denseqmc "$fout"
	else
		cp /tmp/out_denseqmc "$fout"
	fi
	rm -f /tmp/out_denseqmc /tmp/out_sparseqmc /tmp/out_sparseqmcext
	echo
	echo
done

