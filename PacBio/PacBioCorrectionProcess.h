//----------------------------------------------
// Copyright 2015 National Chung Cheng University
// Written by Yao-Ting Huang
// Released under the GPL
//-----------------------------------------------
//
// PacBioCorrectionProcess - Self-correction or hybrid correction using FM-index walk for PacBio reads
//
//

#ifndef PacBioCorrectionProcess_H
#define PacBioCorrectionProcess_H

#include "HashMap.h"
#include "Util.h"
#include "SequenceProcessFramework.h"
#include "SequenceWorkItem.h"
#include "Metrics.h"
#include "BWTIndexSet.h"
#include "SampledSuffixArray.h"
#include "BWTAlgorithms.h"

enum PacBioCorrectionAlgorithm
{
	PBC_SELF,	// PacBio self correction
	PBC_HYBRID	// PacBio Hybrid Correction
};


// Parameter object for the error corrector
struct PacBioCorrectionParameters
{
	PacBioCorrectionAlgorithm algorithm;
	BWTIndexSet indices;

	int numKmerRounds;
	int kmerLength;

	// tree search parameters
	int maxLeaves;
	int minOverlap;
	int maxOverlap;
	

	// PACBIO
	int minKmerLength;
	int FMWKmerThreshold;
	int seedKmerThreshold;
	int downward;
	int collectedSeeds;
	std::vector<int> seedWalkDistance;
};


class PacBioCorrectionResult
{
public:
	PacBioCorrectionResult()
	: kmerize(false),kmerize2(false),merge(false),merge2(false),
	totalReadsLen(0),
	correctedLen(0),
	totalSeedNum(0),
	totalWalkNum(0),
	correctedNum(0),
	highErrorNum(0),
	exceedDepthNum(0),
	exceedLeaveNum(0),
	seedDis(0) {}

	DNAString correctSequence;
	DNAString correctSequence2;

	bool kmerize;
	bool kmerize2;
	bool merge;
	bool merge2;

	size_t kmerLength;
	std::vector<DNAString> kmerizedReads ;
	std::vector<DNAString> kmerizedReads2 ;

	// PacBio reads correction by Ya, v20151001.
	std::vector<DNAString> correctedPacbioStrs;
	int64_t totalReadsLen;
	int64_t correctedLen;
	int64_t totalSeedNum;
	int64_t totalWalkNum;
	int64_t correctedNum;
	int64_t highErrorNum;
	int64_t exceedDepthNum;
	int64_t exceedLeaveNum;
	int64_t seedDis;
};

//
class PacBioCorrectionProcess
{
public:
	PacBioCorrectionProcess(const PacBioCorrectionParameters params);
	~PacBioCorrectionProcess();
	
	PacBioCorrectionResult process(const SequenceWorkItem& workItem)
	{
		switch(m_params.algorithm)
		{
		case PBC_SELF:
			{
				return PBSelfCorrection(workItem);
				break;
			}
		case PBC_HYBRID:
			{
				return PBHybridCorrection(workItem);
				break;
			}

		default:
			{
				std::cout << "Unsupported algorithm\n";
				assert(false);
			}
		}
		PacBioCorrectionResult result;
		return result;
	}		
	// PacBio correction by Ya, v20150305.
	PacBioCorrectionResult PBSelfCorrection(const SequenceWorkItem& workItem);
	PacBioCorrectionResult PBHybridCorrection(const SequenceWorkItem& workItem);

private:

	// PacBio correction by Ya, v20150305.
	std::vector<std::pair<int, std::string> > searchingSeedsUsingSolidKmer(const std::string readSeq);
	std::vector<std::pair<int,std::string> > findSeedsUsingDynamicKmerLen(const std::string readSeq);
	int doubleFMWalkForPacbio(std::pair<int,std::string> firstSeed, std::pair<int,std::string> secondSeed, int minOverlap, int needWalkLen, std::string* mergedseq);
	int solveHighError(std::pair<int,std::string> firstSeed, std::pair<int,std::string> secondSeed, int minOverlap, int needWalkLen, std::string* mergedseq);
	
	PacBioCorrectionParameters m_params;

};

// Write the results from the overlap step to an ASQG file
class PacBioCorrectionPostProcess
{
public:
	PacBioCorrectionPostProcess(std::ostream* pCorrectedWriter,
	std::ostream* pDiscardWriter,
	const PacBioCorrectionParameters params);

	~PacBioCorrectionPostProcess();

	void process(const SequenceWorkItem& item, const PacBioCorrectionResult& result);
	void process(const SequenceWorkItemPair& itemPair, const PacBioCorrectionResult& result);

private:

	std::ostream* m_pCorrectedWriter;
	std::ostream* m_pDiscardWriter;
	PacBioCorrectionParameters m_params;
	
	int64_t m_totalReadsLen;
	int64_t m_correctedLen;
	int64_t m_totalSeedNum;
	int64_t m_totalWalkNum;
	int64_t m_correctedNum;
	int64_t m_highErrorNum;
	int64_t m_exceedDepthNum;
	int64_t m_exceedLeaveNum;
	int64_t m_seedDis;

};

#endif