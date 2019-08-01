

#include <vector>
#include "utils.h"
#include <opencv2/core/core.hpp>

#include "GCRANSAC.h"
#include "fundamental_estimator.h"

#include <ctime>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>

struct stat info;

enum Problem {
	FundamentalMatrixFitting,
	HomographyFitting
};


void testFundamentalMatrixFitting(std::string source_path_,
	std::string destination_path_,
	std::string out_correspondence_path_,
	std::string in_correspondence_path_,
	std::string output_match_image_path_,
	const double confidence_,
	const double inlier_outlier_threshold_,
	const double spatial_coherence_weight_,
	const double neighborhood_size_,
	const int fps_);

std::vector<std::string> getAvailableTestScenes(Problem problem_);

bool initializeScene(const std::string &scene_name_,
	std::string &src_image_path_,
	std::string &dst_image_path_,
	std::string &input_correspondence_path_,
	std::string &output_correspondence_path_,
	std::string &output_matched_image_path_);

int main(int argc, const char* argv[])
{
	srand(static_cast<int>(time(NULL)));

	const double confidence = 0.99; // The RANSAC confidence value
	const int fps = -1; // The required FPS limit. If it is set to -1, the algorithm will not be interrupted before finishing.
	const double inlier_outlier_threshold = 2.00; // The used inlier-outlier threshold in GC-RANSAC.
	const double spatial_coherence_weight = 0.14; // The weight of the spatial coherence term in the graph-cut energy minimization.
	const double neighborhood_size = 20.0; // The radius of the neighborhood ball for determining the neighborhoods.

	printf("------------------------------------------------------------\nFundamental matrix fitting\n------------------------------------------------------------\n");
	for (const std::string &scene : getAvailableTestScenes(Problem::FundamentalMatrixFitting))
	{
		printf("Processed scene = '%s'\n", scene.c_str());
		std::string src_image_path, // Path of the source image
			dst_image_path, // Path of the destination image
			input_correspondence_path, // Path where the detected correspondences are saved
			output_correspondence_path, // Path where the inlier correspondences are saved
			output_matched_image_path; // Path where the matched image is saved

		// Initializing the paths
		initializeScene(scene,
			src_image_path,
			dst_image_path,
			input_correspondence_path,
			output_correspondence_path,
			output_matched_image_path);

		// Estimating the fundamental matrix by the Graph-Cut RANSAC algorithm
		testFundamentalMatrixFitting(
			src_image_path, // The source image's path
			dst_image_path, // The destination image's path
			input_correspondence_path, // The path where the detected correspondences (before the robust estimation) will be saved (or loaded from if exists)
			output_correspondence_path, // The path where the inliers of the estimated fundamental matrices will be saved
			output_matched_image_path, // The path where the matched image pair will be saved
			confidence, // The RANSAC confidence value
			inlier_outlier_threshold, // The used inlier-outlier threshold in GC-RANSAC.
			spatial_coherence_weight, // The weight of the spatial coherence term in the graph-cut energy minimization.
			neighborhood_size, // The radius of the neighborhood ball for determining the neighborhoods.
			fps); // The required FPS limit. If it is set to -1, the algorithm will not be interrupted before finishing.
		printf("\n");
	}

	return 0;
}

std::vector<std::string> getAvailableTestScenes(Problem problem_)
{
	switch (problem_)
	{
	case Problem::FundamentalMatrixFitting:
		return { "head", "johnssona", "Kyoto" };
	case Problem::HomographyFitting:
		return { };
	default:
		return {};
	}
}

bool initializeScene(const std::string &scene_name_,
	std::string &src_image_path_,
	std::string &dst_image_path_,
	std::string &input_correspondence_path_,
	std::string &output_correspondence_path_,
	std::string &output_matched_image_path_)
{
	// The root directory where the "results" and "data" folder are
	const std::string root_dir = "";

	// The directory to which the results will be saved
	std::string dir = root_dir + "results/" + scene_name_;

	// Create the task directory if it doesn't exist
	if (stat(dir.c_str(), &info) != 0) // Check if exists
		if (_mkdir(dir.c_str()) != 0) // Create it, if not
		{
			fprintf(stderr, "Error while creating a new folder in 'results'\n");
			return false;
		}

	// The source image's path
	src_image_path_ =
		root_dir + "data/" + scene_name_ + "/" + scene_name_ + "1.jpg";
	// The destination image's path
	dst_image_path_ =
		root_dir + "data/" + scene_name_ + "/" + scene_name_ + "2.jpg";
	// The path where the detected correspondences (before the robust estimation) will be saved (or loaded from if exists)
	input_correspondence_path_ =
		root_dir + "results/" + scene_name_ + "/" + scene_name_ + "_points_with_no_annotation.txt";
	// The path where the inliers of the estimated fundamental matrices will be saved
	output_correspondence_path_ =
		root_dir + "results/" + scene_name_ + "/result_" + scene_name_ + ".txt";
	// The path where the matched image pair will be saved
	output_matched_image_path_ =
		root_dir + "results/" + scene_name_ + "/matches_" + scene_name_ + ".png";

	return true;
}

void testHomographyFitting(
	std::string source_path_,
	std::string destination_path_,
	std::string out_correspondence_path_,
	std::string in_correspondence_path_,
	std::string output_match_image_path_,
	const double confidence_,
	const double inlier_outlier_threshold_,
	const double spatial_coherence_weight_,
	const double neighborhood_size_,
	const int fps_)
{
	
}

void testFundamentalMatrixFitting(
	std::string source_path_,
	std::string destination_path_,
	std::string out_correspondence_path_,
	std::string in_correspondence_path_,
	std::string output_match_image_path_,
	const double confidence_,
	const double inlier_outlier_threshold_,
	const double spatial_coherence_weight_,
	const double neighborhood_size_,
	const int fps_)
{
	// Read the images
	cv::Mat source_image = cv::imread(source_path_);
	cv::Mat destination_image = cv::imread(destination_path_);

	if (source_image.empty()) // Check if the source image is loaded succesfully
	{
		printf("An error occured while loading image '%s'\n", 
			source_path_.c_str());
		return;
	}

	if (destination_image.empty()) // Check if the destination image is loaded succesfully
	{
		printf("An error occured while loading image '%s'\n", 
			destination_path_.c_str());
		return;
	}

	// Detect or load point correspondences using AKAZE 
	cv::Mat points;
	detectFeatures(
		in_correspondence_path_, 
		source_image, 
		destination_image, 
		points);

	// Apply Graph-cut RANSAC
	FundamentalMatrixEstimator estimator;
	std::vector<int> inliers;
	FundamentalMatrix model;
	size_t iteration_number;

	GCRANSAC<FundamentalMatrixEstimator, FundamentalMatrix> gcransac;
	gcransac.setFPS(fps_); // Set the desired FPS (-1 means no limit)
	gcransac.settings.threshold = inlier_outlier_threshold_; // The inlier-outlier threshold
	gcransac.settings.spatial_coherence_weight = spatial_coherence_weight_; // The weight of the spatial coherence term
	gcransac.settings.confidence = confidence_; // The required confidence in the results
	gcransac.settings.max_local_optimization_number = 20; // The maximm number of local optimizations
	gcransac.settings.max_iteration_number = 5000; // The maximum number of iterations
	gcransac.settings.min_iteration_number = 50; // The minimum number of iterations
	gcransac.settings.neighborhood_sphere_radius = neighborhood_size_; // The radius of the neighborhood ball
	gcransac.settings.core_number = 4; // The number of parallel processes

	// Start GC-RANSAC
	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	gcransac.run(points, 
		estimator,
		model);
	end = std::chrono::system_clock::now();

	// Calculate the processing time
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);
	
	// Get the statistics of the results
	const RANSACStatistics &statistics = gcransac.getRansacStatistics();

	// Write statistics
	printf("Elapsed time = %f secs\n", elapsed_seconds.count());
	printf("Inlier number = %d\n", static_cast<int>(statistics.inliers.size()));
	printf("Applied number of local optimizations = %d\n", statistics.local_optimization_number);
	printf("Applied number of graph-cuts = %d\n", statistics.graph_cut_number);
	printf("Number of iterations = %d\n\n", statistics.iteration_number);

	// Draw the inlier matches to the images
	cv::Mat match_image;
	drawMatches(points, 
		statistics.inliers,
		source_image, 
		destination_image, 
		match_image);

	printf("Saving the matched images to file '%s'.\n", output_match_image_path_.c_str());
	imwrite(output_match_image_path_, match_image); // Save the matched image to file
	printf("Saving the inlier correspondences to file '%s'.\n", out_correspondence_path_.c_str());
	savePointsToFile(points, out_correspondence_path_.c_str(), &statistics.inliers); // Save the inliers to file
	
	printf("Press a button to continue...\n");

	// Showing the image
	showImage(match_image,
		"Inlier correspondences",
		1600,
		1200,
		true);
}
