#pragma once

#include "BamProcessorAbstract.h"
#include <Estimation/CellsDataContainer.h>

#include <unordered_map>
#include <string>

namespace Estimation
{
	namespace BamProcessing
	{
		class FilteringBamProcessor : public BamProcessorAbstract
		{
		private:
			using cb_map_t = std::unordered_map<std::string, std::string>;

			cb_map_t merge_cbs;
			bool is_bam_open;
			size_t written_reads;

			const CellsDataContainer &_container;

		protected:
			std::string get_result_bam_name(const std::string &bam_name) const override;

		public:
			FilteringBamProcessor(const BamTags &tags, const CellsDataContainer &container);

			void update_bam(const std::string& bam_file, const BamTools::BamReader &reader) override;
			void trace_state(const std::string& bam_file) const override;
			void save_read(const std::string& cell_barcode, const std::string& chr_name, const std::string& umi,
								   const std::string& gene, const UMI::Mark &umi_mark) override;
			void write_alignment(BamTools::BamAlignment alignment, const std::string& gene,
										 const Tools::ReadParameters &read_params) override;

			const CellsDataContainer& container() const override;
		};
	}
}
