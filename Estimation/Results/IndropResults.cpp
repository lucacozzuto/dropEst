#include "IndropResults.h"

namespace Estimation
{
	namespace Results
	{
		IndropResult::IndropResult(const CountMatrix &cm, const Stats &stats, const std::vector<double> &reads_per_umi,
		                           const int_list_t &umig_covered, bool not_filtered)
				: cm(cm), reads_per_umi(reads_per_umi), umig_covered(umig_covered), merge_n(stats.get_merge_counts()),
				  reads_by_umig(stats.get(Stats::READS_BY_UMIG)), reads_by_cb(stats.get(Stats::READS_BY_CB))
		{
			if (not_filtered)
			{
				stats.get_cells(Stats::EXONE_UMI_PER_CELL_PER_CHR, this->ex_cell_names, this->chr_names,
				                this->ex_cells_chr_umis_counts);
				this->chr_names.clear();
				stats.get_cells(Stats::NON_EXONE_UMI_PER_CELL_PER_CHR, this->nonex_cell_names, this->chr_names,
				                this->nonex_cells_chr_umis_counts);
			}
			else
			{
				stats.get_cells_filtered(Stats::EXONE_UMI_PER_CELL_PER_CHR, this->cm.cell_names, this->ex_cell_names,
				                         this->chr_names,
				                         this->ex_cells_chr_umis_counts);
				this->chr_names.clear();
				stats.get_cells_filtered(Stats::NON_EXONE_UMI_PER_CELL_PER_CHR, this->cm.cell_names,
				                         this->nonex_cell_names,
				                         this->chr_names, this->nonex_cells_chr_umis_counts);
			}
		}

#ifdef R_LIBS
		void IndropResult::save_r_table(const std::string &filename) const
		{
			using namespace Rcpp;

			RInside R(0, 0);
			R["d"] = List::create(Named("cell.names") = wrap(this->cm.cell_names),
			                    Named("gene.names") = wrap(this->cm.gene_names),
			                    Named("cm") = wrap(this->cm.counts),
			                    Named("ex_cells_chr_counts") = wrap(this->ex_cells_chr_umis_counts),
			                    Named("nonex_cells_chr_counts") = wrap(this->nonex_cells_chr_umis_counts),
			                    Named("ex_counts_cell_names") = wrap(this->ex_cell_names),
			                    Named("nonex_counts_cell_names") = wrap(this->nonex_cell_names),
			                    Named("counts_chr_names") = wrap(this->chr_names),
			                    Named("rpu") = wrap(this->reads_per_umi),
			                    Named("umig.cov") = wrap(this->umig_covered),
			                    Named("merge.n") = wrap(this->merge_n),
			                    Named("reads_by_umig") = wrap(this->reads_by_umig),
			                    Named("reads_by_cb") = wrap(this->reads_by_cb),
			                    Named("fname") = wrap(fname));
			L_TRACE << "writing R data to " << output_name;

			R.parseEvalQ(
					"d$ex_cells_chr_counts<-as.data.frame(matrix(d$ex_cells_chr_counts, length(d$ex_counts_cell_names), "
							"length(d$counts_chr_names), byrow = TRUE), row.names = d$ex_counts_cell_names); "
							"colnames(d$ex_cells_chr_counts)<-d$counts_chr_names; d$ex_counts_cell_names<-NULL;");

			R.parseEvalQ(
					"d$nonex_cells_chr_counts<-as.data.frame(matrix(d$nonex_cells_chr_counts, length(d$nonex_counts_cell_names), "
							"length(d$counts_chr_names), byrow = TRUE), row.names = d$nonex_counts_cell_names); "
							"colnames(d$nonex_cells_chr_counts)<-d$counts_chr_names; d$nonex_counts_cell_names<-NULL;"
							"d$counts_chr_names<-NULL;");

			R.parseEvalQ("saveRDS(d, '" + filename + "')");
			L_TRACE << "Done";
		}
#endif
	}
}