#!/bin/sh -e
fail() {
    echo "Error: $1"
    exit 1
}

notExists() {
   [ ! -f "$1" ]
}

abspath() {
    if [ -d "$1" ]; then
        echo "$(cd "$1"; pwd)"
    elif [ -f "$1" ]; then
        if [ -z "${1##*/*}" ]; then
            echo "$(cd "${1%/*}"; pwd)/${1##*/}"
        else
            echo "$(pwd)/$1"
        fi
    elif [ -d "$(dirname "$1")" ]; then
        echo "$(cd "$(dirname "$1")"; pwd)/$(basename "$1")"
    fi
}

# check number of input variables
[ "$#" -ne 3 ] && echo "Please provide <sequenceFASTA> <outFile> <tmp>" && exit 1;
# check paths
[ ! -f "$1" ] &&  echo "$1 not found!" && exit 1;
[   -f "$2" ] &&  echo "$2 exists already!" && exit 1;
[ ! -d "$3" ] &&  echo "tmp directory $3 not found!" && mkdir -p "$3";

INPUT="$(abspath "$1")"
RESULTS="$(abspath "$2")"
TMP_PATH="$(abspath "$3")"

if notExists "${TMP_PATH}/query"; then
   "$MMSEQS" createdb "${INPUT}" "${TMP_PATH}/input" \
        || fail "query createdb died"
fi

INTERMEDIATE="${TMP_PATH}/clu"
if notExists "${TMP_PATH}/clu"; then
    # shellcheck disable=SC2086
    "$MMSEQS" cluster "${TMP_PATH}/input" "${TMP_PATH}/clu" "${TMP_PATH}/clu_tmp" ${CLUSTER_PAR} \
        || fail "Search died"
fi


if notExists "${TMP_PATH}/cluster.tsv"; then
    # shellcheck disable=SC2086
    "$MMSEQS" createtsv "${TMP_PATH}/input" "${TMP_PATH}/input" "${TMP_PATH}/clu" "${TMP_PATH}/cluster.tsv"  \
        || fail "Convert Alignments died"
fi

if notExists "${TMP_PATH}/req_seq.fasta"; then
    "$MMSEQS" result2repseq "${TMP_PATH}/input" "${TMP_PATH}/clu" "${TMP_PATH}/clu_rep" \
            || fail "Result2repseq  died"

    "$MMSEQS" result2flat "${TMP_PATH}/input" "${TMP_PATH}/input" "${TMP_PATH}/clu_rep" "${TMP_PATH}/req_seq.fasta" --use-fasta-header \
            || fail "result2flat died"
fi

if notExists "${TMP_PATH}/all_seqs.fasta"; then
    "$MMSEQS" createseqfiledb "${TMP_PATH}/input" "${TMP_PATH}/clu" "${TMP_PATH}/clu_seqs" \
            || fail "Result2repseq  died"

    "$MMSEQS" result2flat "${TMP_PATH}/input" "${TMP_PATH}/input" "${TMP_PATH}/clu_seqs" "${TMP_PATH}/all_seqs.fasta" \
            || fail "result2flat died"
fi

mv "${TMP_PATH}/all_seqs.fasta"  "${RESULTS}_all_seqs.fasta"
mv "${TMP_PATH}/req_seq.fasta"  "${RESULTS}_req_seq.fasta"
mv "${TMP_PATH}/cluster.tsv"  "${RESULTS}_cluster.tsv"



if [ -n "${REMOVE_TMP}" ]; then
    echo "Removing temporary files"
    rm -f "${TMP_PATH}/input" "${TMP_PATH}/input.index"
    rm -f "${TMP_PATH}/clu_seqs" "${TMP_PATH}/clu_seqs.index"
    rm -f "${TMP_PATH}/clu_rep" "${TMP_PATH}/clu_rep.index"
    rm -f "${TMP_PATH}/clu" "${TMP_PATH}/clu.index"
    rm -rf "${TMP_PATH}/clu_tmp"
    rm -f "${TMP_PATH}/easycluster.sh"
fi