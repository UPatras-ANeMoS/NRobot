/*
	Copyright (C) 2016-2017 Sotiris Papatheodorou

	This file is part of NRobot.

    NRobot is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NRobot is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with NRobot.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "NRPart.hpp"
#include "NRClip.hpp"



/************************************************************/
/********************* Helper functions *********************/
/************************************************************/
nr::Polygon nr_halfplane(
	const nr::Point& A,
	const nr::Point& B,
	const double diam
) {
	/* Initialize halfplane */
	nr::Polygon H;
	H.contour.resize(1);
	H.is_hole.resize(1);
	H.is_open.resize(1);
	H.is_hole[0] = false;
	H.is_open[0] = false;
	H.contour[0].resize(4);

	/* Create halfplane assuming both points are on x axis with center on origin */
	H.contour[0][0].x = 0;
	H.contour[0][0].y = -diam;
	H.contour[0][1].x = -diam;
	H.contour[0][1].y = -diam;
	H.contour[0][2].x = -diam;
	H.contour[0][2].y = diam;
	H.contour[0][3].x = 0;
	H.contour[0][3].y = diam;

	/* Rotate halfplane */
	double theta = std::atan2(B.y-A.y, B.x-A.x);
	nr::rotate( &H, theta, true);

	/* Translate halfplane */
	nr::translate( &H, nr::midpoint(A,B) );

	return H;
}

nr::Polygon nr_hyperbola_branch_old(
	const nr::Circle& A,
	const nr::Circle& B,
	const double diam,
	const size_t ppb
) {
	/* Initialize hyperbola branch */
	nr::Polygon H;


	if (nr::is_point(A) && nr::is_point(B)) {
		/* If both circles are actually points create the classic voronoi */
		H.contour.resize(1);
		H.is_hole.resize(1);
		H.is_open.resize(1);
		H.is_hole[0] = false;
		H.is_open[0] = false;
		H.contour[0].resize(4);

		H.contour[0][0].x = 0;
		H.contour[0][0].y = -diam;
		H.contour[0][1].x = -diam;
		H.contour[0][1].y = -diam;
		H.contour[0][2].x = -diam;
		H.contour[0][2].y = diam;
		H.contour[0][3].x = 0;
		H.contour[0][3].y = diam;
	} else {
		/* Create hyperbola branch assuming both foci are on x axis with center on origin */
		double a = (A.radius + B.radius) / 2;
		double c = nr::dist(A.center, B.center) / 2;
		double b = std::sqrt( c*c - a*a );

		if ( c < a) {
			/* The disks overlap and there is no cell */
			nr::make_empty(&H);
			return H;
		} else {
			/* Create all hyperbola branch points using the parametric equation */
			H.contour.resize(1);
			H.is_hole.resize(1);
			H.is_open.resize(1);
			H.is_hole[0] = false;
			H.is_open[0] = false;
			H.contour[0].resize(ppb);

			/* Parameter step */
			double dt = 2*std::acosh(diam/a)/ppb;
			/* Initial parameter value. Parameter range [-pi/2, pi/2] */
			double t = -std::acosh(diam/a) - dt;

			for (size_t i=0; i<ppb; i++) {
				t = t + dt;
				H.contour[0][i].x = -a * std::cosh(t);
				H.contour[0][i].y = b * std::sinh(t);
			}
		}
	}

	/* Rotate halfplane */
	double theta = std::atan2(B.center.y-A.center.y, B.center.x-A.center.x);
	nr::rotate( &H, theta, true);

	/* Translate halfplane */
	nr::translate( &H, nr::midpoint(A.center ,B.center) );

	return H;
}

nr::Polygon nr_hyperbola_branch(
	const nr::Point& A,
	const nr::Point& B,
	const double a,
	const double c,
	const double diam,
	const size_t ppb
) {
	/*
	   Assume A = [-c 0]^T and B = [c 0]^T. Thus the hyperbola parameterization
	   is x = +-a*cosh(t) and y = b*sinh(t).

	   For a > 0, the branch of A is the left branch of the hyperbola,
	   given by-a*cosh(t).
	   For a < 0, the branch of A is the right branch of the hyperbola and
	   since a < 0 it is also given by -a*cosh(t).

	   After computing the hyperbola branch, rotate it by the angle of the
	   vector B-A and translate it by the vector (A+B)/2.
	*/

	/* Initialize hyperbola branch */
	nr::Polygon H;

	if ( a >= c ) {
		/* The cell of A with respect to B is empty */
		nr::make_empty(&H);
	} else if ( a <= -c ) {
		/* The cell of A with respect to B is the whole region */
		/* Initialize polygon */
		H.contour.resize(1);
		H.is_hole.resize(1);
		H.is_open.resize(1);
		H.is_hole[0] = false;
		H.is_open[0] = false;
		H.contour[0].resize(4);
		/* Create large rectangle assuming both points are on x axis with center on origin */
		H.contour[0][0].x = diam;
		H.contour[0][0].y = -diam;
		H.contour[0][1].x = -diam;
		H.contour[0][1].y = -diam;
		H.contour[0][2].x = -diam;
		H.contour[0][2].y = diam;
		H.contour[0][3].x = diam;
		H.contour[0][3].y = diam;
	} else if ( std::abs(a) <= NR_CMP_ERR ) {
		/* The cell of A with respect to B is a halfplane */
		H = nr_halfplane( A, B, diam );
	} else if (a > 0) {
		/* The convex cell of A with respect to B is bound by a hyperbola branch */
		double b = std::sqrt( c*c - a*a );
		/* Create all hyperbola branch points using the parametric equation */
		H.contour.resize(1);
		H.is_hole.resize(1);
		H.is_open.resize(1);
		H.is_hole[0] = false;
		H.is_open[0] = false;
		H.contour[0].resize(ppb);

		/* Parameter step */
		double dt = 2*std::acosh(diam/std::abs(a))/ppb;
		/* Initial parameter value. Parameter range [-pi/2, pi/2] */
		double t = -std::acosh(diam/std::abs(a)) - dt;

		for (size_t i=0; i<ppb; i++) {
			t = t + dt;
			H.contour[0][i].x = -a * std::cosh(t);
			H.contour[0][i].y = b * std::sinh(t);
		}

		/* Rotate cell */
		double theta = std::atan2(B.y-A.y, B.x-A.x);
		nr::rotate( &H, theta, true);

		/* Translate cell */
		nr::translate( &H, nr::midpoint(A ,B) );
	} else if (a < 0) {
		/* The non convex cell of A with respect to B is bound by a hyperbola branch */
		double b = std::sqrt( c*c - a*a );
		/* Create all hyperbola branch points using the parametric equation */
		H.contour.resize(1);
		H.is_hole.resize(1);
		H.is_open.resize(1);
		H.is_hole[0] = false;
		H.is_open[0] = false;
		/* Allocate space for the 4 extra vertices needed to form the non convex cell */
		H.contour[0].resize(ppb+4);

		/* Parameter step */
		double dt = 2*std::acosh(diam/std::abs(a))/ppb;
		/* Initial parameter value. Parameter range [-pi/2, pi/2] */
		double t = -std::acosh(diam/std::abs(a)) - dt;

		for (size_t i=0; i<ppb; i++) {
			t = t + dt;
			H.contour[0][i].x = -a * std::cosh(t);
			H.contour[0][i].y = b * std::sinh(t);
		}

		/* Vector from j to i and vector from j to i rotated -90 degrees */
		nr::Point BA, BAr;
		BA = nr::Point( A.x-B.x, A.y-B.y );
		BA = -BA; /* I don't know why this is needed yet */
		BAr = nr::rotate( BA, -M_PI/2 );

		/* Add the four extra vertices needed to form the non convex cell */
		H.contour[0][ppb] = H.contour[0][ppb-1] + diam*BAr;
		H.contour[0][ppb+1] = H.contour[0][ppb] + diam*BA;
		H.contour[0][ppb+3] = H.contour[0][0] - diam*BAr;
		H.contour[0][ppb+2] = H.contour[0][ppb+3] + diam*BA;

		/* Rotate cell */
		double theta = std::atan2(B.y-A.y, B.x-A.x);
		nr::rotate( &H, theta, true);

		/* Translate cell */
		nr::translate( &H, nr::midpoint(A ,B) );
	}

	return H;
}






/**********************************************************/
/********************* Main functions *********************/
/**********************************************************/

int nr::voronoi(
	const nr::Polygon& region,
	const nr::Points& seeds,
	nr::Polygons* cells
) {
	/* Number of seeds */
	size_t N = seeds.size();
	/* Initialize the result */
	cells->resize(N);
	/* Region diameter */
	double diam = nr::diameter(region);

	/* Loop over all seed pairs */
	for (size_t i=0; i<N; i++) {
		/* Initialize the cell of i to the region */
		(*cells)[i] = region;
		for (size_t j=0; j<N; j++) {
			if (i != j) {
				/* Create the halfplane containing i with respect to j */
				nr::Polygon h = nr_halfplane( seeds[i], seeds[j], diam );
				/* Intersect the current cell with the halfplane with j */
				int err = nr::polygon_clip( nr::AND, (*cells)[i], h, &((*cells)[i]) );
				if (err) {
					std::printf("Clipping operation returned error %d\n", err);
					return nr::ERROR_PARTITIONING_FAILED;
				}
			}
		}
	}
	return nr::SUCCESS;
}

int nr::voronoi_cell(
	const nr::Polygon& region,
	const nr::Points& seeds,
	const size_t subject,
	nr::Polygon* cell
) {
	/* Number of seeds */
	size_t N = seeds.size();
	/* Region diameter */
	double diam = nr::diameter(region);
	/* Initialize the subject cell to the region */
	(*cell) = region;
	/* Loop over all other seeds */
	for (size_t j=0; j<N; j++) {
		if (subject != j) {
			/* Create the halfplane with respect to j containing subject */
			nr::Polygon h = nr_halfplane( seeds[subject], seeds[j], diam );
			/* Intersect the current cell with the halfplane with respect to j */
			int err = nr::polygon_clip( nr::AND, *cell, h, cell );
			if (err) {
				std::printf("Clipping operation returned error %d\n", err);
				return nr::ERROR_PARTITIONING_FAILED;
			}
		}
	}
	return nr::SUCCESS;
}

int nr::g_voronoi(
	const nr::Polygon& region,
	const nr::Circles& seeds,
	nr::Polygons* cells,
	const size_t points_per_branch
) {
	/* Number of seeds */
	size_t N = seeds.size();
	/* Initialize the result */
	cells->resize(N);
	/* Region diameter */
	double diam = nr::diameter(region);

	/* Loop over all seed pairs */
	for (size_t i=0; i<N; i++) {
		/* Initialize the cell of i to the region */
		(*cells)[i] = region;
		for (size_t j=0; j<N; j++) {
			if (i != j) {
				/* Hyperbola parameters */
				double a = (seeds[i].radius + seeds[j].radius) / 2;
				double c = nr::dist(seeds[i].center, seeds[j].center) / 2;
				/* Create the hyperbola branch containing i with respect to j */
				nr::Polygon h = nr_hyperbola_branch( seeds[i].center, seeds[j].center, a, c, diam, points_per_branch );
				// nr::Polygon h = nr_hyperbola_branch_old( seeds[i], seeds[j], diam, points_per_branch );
				/* Intersect the current cell with the branch with j */
				int err = nr::polygon_clip( nr::AND, (*cells)[i], h, &((*cells)[i]) );
				if (err) {
					std::printf("Clipping operation returned error %d\n", err);
					return nr::ERROR_PARTITIONING_FAILED;
				}
			}
		}
	}
	return nr::SUCCESS;
}

int nr::g_voronoi_cell(
	const nr::Polygon& region,
	const nr::Circles& seeds,
	const size_t subject,
	nr::Polygon* cell,
	const size_t points_per_branch
) {
	/* Number of seeds */
	size_t N = seeds.size();
	/* Region diameter */
	double diam = nr::diameter(region);
	/* Initialize the subject cell to the region */
	(*cell) = region;
	/* Loop over all other seeds */
	for (size_t j=0; j<N; j++) {
		if (subject != j) {
			/* Hyperbola parameters */
			double a = (seeds[subject].radius + seeds[j].radius) / 2;
			double c = nr::dist(seeds[subject].center, seeds[j].center) / 2;
			/* Create the hyperbola branch with respect to j containing subject */
			nr::Polygon h = nr_hyperbola_branch( seeds[subject].center, seeds[j].center, a, c, diam, points_per_branch );
			// nr::Polygon h = nr_hyperbola_branch_old( seeds[subject], seeds[j], diam, points_per_branch );
			/* Intersect the current cell with the branch with respect to j */
			int err = nr::polygon_clip( nr::AND, *cell, h, cell );
			if (err) {
				std::printf("Clipping operation returned error %d\n", err);
				return nr::ERROR_PARTITIONING_FAILED;
			}
		}
	}
	return nr::SUCCESS;
}

int nr::awg_voronoi_cell(
	const nr::Polygon& region,
	const nr::Circles& seeds,
	const std::vector<double>& weights,
	const size_t subject,
	nr::Polygon* cell,
	const size_t points_per_branch
) {
	/* Number of seeds */
	size_t N = seeds.size();
	/* Region diameter */
	double diam = nr::diameter(region);
	/* Initialize the subject cell to the region */
	(*cell) = region;
	/* Loop over all other seeds */
	for (size_t j=0; j<N; j++) {
		if (subject != j) {
			/* Hyperbola parameters */
			double a = (seeds[subject].radius + seeds[j].radius + weights[j] - weights[subject]) / 2;
			double c = nr::dist(seeds[subject].center, seeds[j].center) / 2;

			/* Create the hyperbola branch with respect to j containing subject */
			nr::Polygon h = nr_hyperbola_branch( seeds[subject].center, seeds[j].center, a, c, diam, points_per_branch );
			/* Intersect the current cell with the branch with respect to j */
			int err = nr::polygon_clip( nr::AND, *cell, h, cell );
			if (err) {
				std::printf("Clipping operation returned error %d\n", err);
				return nr::ERROR_PARTITIONING_FAILED;
			}
		}
	}
	return nr::SUCCESS;
}

int nr::ys_partitioning(
	const nr::Polygon& region,
	const nr::Polygons& seeds,
	nr::Polygons* cells
) {
	/* return value of clipping operations */
	int err;
	/* Number of seeds */
	size_t N = seeds.size();
	/* Initialize the result */
	cells->resize(N+1);
	/* Initialize the common sensed region */
	nr::make_empty( &((*cells)[N]) );

	/* Loop over all seed pairs */
	for (size_t i=0; i<N; i++) {
		/* Initialize the cell of i to the region */
		(*cells)[i] = seeds[i];
		for (size_t j=0; j<N; j++) {
			if (i != j) {
				/* Remove the pattern of j from i */
				err = nr::polygon_clip( nr::DIFF, (*cells)[i], seeds[j], &((*cells)[i]) );
				if (err) {
					std::printf("Clipping operation returned error %d\n", err);
					return nr::ERROR_PARTITIONING_FAILED;
				}
				/* Add the overlapping between i and j to the common sensed region */
				nr::Polygon tmpP;
				err = nr::polygon_clip( nr::AND, seeds[i], seeds[j], &tmpP );
				if (err) {
					std::printf("Clipping operation returned error %d\n", err);
					return nr::ERROR_PARTITIONING_FAILED;
				}
				err = nr::polygon_clip( nr::OR, (*cells)[N], tmpP, &((*cells)[N]) );
				if (err) {
					std::printf("Clipping operation returned error %d\n", err);
					return nr::ERROR_PARTITIONING_FAILED;
				}
			}
		}
		/* Intersect the cell with the region in order to constrain it */
		err = nr::polygon_clip( nr::AND, (*cells)[i], region, &((*cells)[i]) );
		if (err) {
			std::printf("Clipping operation returned error %d\n", err);
			return nr::ERROR_PARTITIONING_FAILED;
		}
	}

	/* Intersect the common sensed region with the region */
	err = nr::polygon_clip( nr::AND, (*cells)[N], region, &((*cells)[N]) );
	if (err) {
		std::printf("Clipping operation returned error %d\n", err);
		return nr::ERROR_PARTITIONING_FAILED;
	}

	return nr::SUCCESS;
}

int nr::ysuq_partitioning(
	const nr::Polygon& region,
	const nr::Circles& seeds,
	const std::vector<double>& quality,
	nr::Polygons* cells,
	bool **neighbors
) {
	/* return value of clipping operations */
	int err;
	/* Number of seeds */
	size_t N = seeds.size();
	/* Initialize the result */
	cells->resize(N);
	/* Initialize the neighbors array if required */
	if (neighbors) {
		free(*neighbors);
		*neighbors = (bool*) malloc( N*N*sizeof(bool) );
	}
	/* Convert the circles to polygons */
	nr::Polygons sensing;
	sensing.resize(N);
	for (size_t i=0; i<N; i++) {
		sensing = nr::Polygons(seeds);
	}

	/* Loop over all nodes */
	for (size_t i=0; i<N; i++) {
		/* Initialize cells to seed polygons */
		(*cells)[i] = sensing[i];

		/* Loop over all other nodes */
		for (size_t j=0; j<N; j++) {
			if (i != j) {
				/* Check for overlapping of the sensing disks */
				if (nr::dist(seeds[i].center, seeds[j].center) <= seeds[i].radius + seeds[j].radius) {
					/* Compare coverage quality */
					if (quality[i] < quality[j]) {
						/* Remove the seed polygon of j */
						err = nr::polygon_clip(nr::DIFF, (*cells)[i], sensing[j], &((*cells)[i]));
						if (err) {
							std::printf("Clipping operation returned error %d\n", err);
							return nr::ERROR_PARTITIONING_FAILED;
						}
					} else if (quality[i] == quality[j]) {
						/* Use the arbitrary partitioning */
						nr::Polygon H = nr_halfplane( seeds[j].center, seeds[i].center, seeds[j].radius );
						err = nr::polygon_clip(nr::DIFF, (*cells)[i], H, &((*cells)[i]));
						if (err) {
							std::printf("Clipping operation returned error %d\n", err);
							return nr::ERROR_PARTITIONING_FAILED;
						}
					}

					/* Set neighbor status if required */
					if (neighbors) {
						(*neighbors)[i+N*j] = true;
					}
				} else {
					/* Set neighbor status if required */
					if (neighbors) {
						(*neighbors)[i+N*j] = false;
					}
				}
			} else {
				/* Set neighbor status if required */
				if (neighbors) {
					(*neighbors)[i+N*j] = false;
				}
			}
		}
		/* Intersect the cell with the region */
		err = nr::polygon_clip(nr::AND, (*cells)[i], region, &((*cells)[i]));
		if (err) {
			std::printf("Clipping operation returned error %d\n", err);
			return nr::ERROR_PARTITIONING_FAILED;
		}
	}
	return nr::SUCCESS;
}
