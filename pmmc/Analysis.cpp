#include <iostream>
#include <math.h>
#include "pmmc.h"
#include "Domain.h"
//#include "PointList.h"
//#include "Array.h"

#define CAPRAD 25
#define RADIUS 20
#define SPEED 1.0

using namespace std;

int main(int argc, char **argv)
{
	//.......................................................................
	//  printf("Radius = %s \n,"RADIUS);  
	int Nx,Ny,Nz,N;
	int i,j,k,p,q,r,n;
	int nspheres;
	double Lx,Ly,Lz;
	//.......................................................................
	Nx = Ny = Nz = 60;
	cout << "Enter Domain size " << endl;
	cout << "Nx = " << endl;
	cin >> Nx;
	Ny = Nz = Nx;
	N = Nx*Ny*Nz;
	//.......................................................................
	// Reading the domain information file
/*	//.......................................................................
	ifstream domain("Domain.in");
	domain >> Nx;
	domain >> Ny;
	domain >> Nz;
	domain >> nspheres;
	domain >> Lx;
	domain >> Ly;
	domain >> Lz;
*/	//.......................................................................
	
	//.......................................................................
	DoubleArray SignDist(Nx,Ny,Nz);
	DoubleArray Phase(Nx,Ny,Nz);
	DoubleArray Phase_x(Nx,Ny,Nz);
	DoubleArray Phase_y(Nx,Ny,Nz);
	DoubleArray Phase_z(Nx,Ny,Nz);
	DoubleArray Sx(Nx,Ny,Nz);
	DoubleArray Sy(Nx,Ny,Nz);
	DoubleArray Sz(Nx,Ny,Nz);
	DoubleArray Vel_x(Nx,Ny,Nz);
	DoubleArray Vel_y(Nx,Ny,Nz);
	DoubleArray Vel_z(Nx,Ny,Nz);
	DoubleArray Press(Nx,Ny,Nz);
	DoubleArray GaussCurvature(Nx,Ny,Nz);
	DoubleArray MeanCurvature(Nx,Ny,Nz);
	//.......................................................................

	//.......................................................................
	double fluid_isovalue = 0.0;
	double solid_isovalue = 0.0;
	//.......................................................................

/*	//.......................................................................
	double *cx,*cy,*cz,*rad;
	cx = new double[nspheres];
	cy = new double[nspheres];
	cz = new double[nspheres];
	rad = new double[nspheres];
	//...............................
	printf("Reading the sphere packing \n");
	ReadSpherePacking(nspheres,cx,cy,cz,rad);
	//.......................................................................
	//.......................................................................
	// Compute the signed distance function for the sphere packing
	SignedDistance(SignDist.data,nspheres,cx,cy,cz,rad,Lx,Ly,Lz,Nx,Ny,Nz,0,0,0,1,1,1);
*/	//.......................................................................

	/* ****************************************************************
	 VARIABLES FOR THE PMMC ALGORITHM
	 ****************************************************************** */
	//...........................................................................
	// Averaging variables
	//...........................................................................
	double awn,ans,aws,lwns,nwp_volume;
	double sw,vol_n,vol_w,paw,pan;
	double efawns,Jwn;
	double As;
	double dEs,dAwn,dAns;			 // Global surface energy (calculated by rank=0)
	double awn_global,ans_global,aws_global,lwns_global,nwp_volume_global;	
	double As_global;
	//	bool add=1;			// Set to false if any corners contain nw-phase ( F > fluid_isovalue)
	
	int n_nw_pts=0,n_ns_pts=0,n_ws_pts=0,n_nws_pts=0, map=0;
	int n_nw_tris=0, n_ns_tris=0, n_ws_tris=0, n_nws_seg=0;
	
	double s,s1,s2,s3;		// Triangle sides (lengths)
	Point A,B,C,P;
	//	double area;
	int cube[8][3] = {{0,0,0},{1,0,0},{0,1,0},{1,1,0},{0,0,1},{1,0,1},{0,1,1},{1,1,1}};  // cube corners
	//	int count_in=0,count_out=0;
	//	int nodx,nody,nodz;
	// initialize lists for vertices for surfaces, common line
	DTMutableList<Point> nw_pts(20);
	DTMutableList<Point> ns_pts(20);
	DTMutableList<Point> ws_pts(20);
	DTMutableList<Point> nws_pts(20);
	// initialize triangle lists for surfaces
	IntArray nw_tris(3,20);
	IntArray ns_tris(3,20);
	IntArray ws_tris(3,20);
	// initialize list for line segments
	IntArray nws_seg(2,20);
	DTMutableList<Point> tmp(20);
	
	// Initialize arrays for local solid surface
	DTMutableList<Point> local_sol_pts(20);
	int n_local_sol_pts = 0;
	IntArray local_sol_tris(3,18);
	int n_local_sol_tris;
	DoubleArray values(20);
	DTMutableList<Point> local_nws_pts(20);
	int n_local_nws_pts;
	
	DoubleArray CubeValues(2,2,2);
	DoubleArray ContactAngle(20);
	DoubleArray Curvature(20);
	DoubleArray InterfaceSpeed(20);
	DoubleArray NormalVector(60);
	DoubleArray van(3);
	DoubleArray vaw(3);
	DoubleArray vawn(3);
	DoubleArray Gwn(6);
	DoubleArray Gns(6);
	DoubleArray Gws(6);
	
	double iVol = 1.0/Nx/Ny/Nz;
	
	int c;
	//...........................................................................
	int ncubes = (Nx-2)*(Ny-2)*(Nz-2);	// Exclude the "upper" halo
	IntArray cubeList(3,ncubes);
	pmmc_CubeListFromMesh(cubeList, ncubes, Nx, Ny, Nz);
	//...........................................................................
	double Cx,Cy,Cz;
	double dist1,dist2;
	// Extra copies of phase indicator needed to compute time derivatives on CPU
	DoubleArray Phase_tminus(Nx,Ny,Nz);
	DoubleArray Phase_tplus(Nx,Ny,Nz);
	DoubleArray dPdt(Nx,Ny,Nz);
	Cx = Cy = Cz = Nz*0.5;
	for (k=0; k<Nz; k++){
		for (j=0; j<Ny; j++){
			for (i=0; i<Nx; i++){
				dist2 = sqrt((i-Cx)*(i-Cx)+(j-Cy)*(j-Cy)+(k-Cz)*(k-Cz)) - CAPRAD;

				Phase_tminus(i,j,k) = dist2;
			}
		} 
	}
	Cz += SPEED;
	for (k=0; k<Nz; k++){
		for (j=0; j<Ny; j++){
			for (i=0; i<Nx; i++){
				
				dist1 = sqrt((i-Cx)*(i-Cx)+(j-Cy)*(j-Cy)) - RADIUS;
				dist2 = sqrt((i-Cx)*(i-Cx)+(j-Cy)*(j-Cy)+(k-Cz)*(k-Cz)) - CAPRAD;

				//SignDist(i,j,k) = -dist1;
				//Phase(i,j,k) = dist2;
			}
		}   
	}
	Cz += SPEED;
	for (k=0; k<Nz; k++){
		for (j=0; j<Ny; j++){
			for (i=0; i<Nx; i++){
				dist2 = sqrt((i-Cx)*(i-Cx)+(j-Cy)*(j-Cy)+(k-Cz)*(k-Cz)) - CAPRAD;

				Phase_tplus(i,j,k) = dist2;
			}
		}   
	}
	
	awn = aws = ans = lwns = 0.0;
	nwp_volume = 0.0;
	As = 0.0;
	Jwn = 0.0;
	efawns = 0.0;
	// Compute phase averages
	pan = paw = 0.0;
	vaw(0) = vaw(1) = vaw(2) = 0.0;
	van(0) = van(1) = van(2) = 0.0;
	vawn(0) = vawn(1) = vawn(2) = 0.0;
	Gwn(0) = Gwn(1) = Gwn(2) = 0.0;
	Gwn(3) = Gwn(4) = Gwn(5) = 0.0;
	Gws(0) = Gws(1) = Gws(2) = 0.0;
	Gws(3) = Gws(4) = Gws(5) = 0.0;
	Gns(0) = Gns(1) = Gns(2) = 0.0;
	Gns(3) = Gns(4) = Gns(5) = 0.0;
	vol_w = vol_n =0.0;
	
	// Read the input files for the phase, distance and pressure field
	char PHASEFILE[16];
	sprintf(PHASEFILE,"Phase.in");
	ReadBinaryFile(PHASEFILE,Phase.data,Nx*Ny*Nz);
	char DISTFILE[16];
	sprintf(DISTFILE,"SignDist.in");
	ReadBinaryFile(DISTFILE,SignDist.data,Nx*Ny*Nz);
	/*	FILE *PRESS
	PRESS = fopen("Pressure.in","wb");
	fread(Phase.data,8,N,PRESS);
	fclose(PRESS);
	
	FILE *VEL;
	VEL = fopen("Pressure.in","wb");
	fread(Phase.data,8,3*N,VEL);
	fclose(VEL);
*/	//...........................................................................
	// Calculate the time derivative of the phase indicator field
	
	for (int n=0; n<Nx*Ny*Nz; n++)	dPdt(n) = 0.5*(Phase_tplus(n) - Phase_tminus(n));
	
	pmmc_MeshGradient(Phase,Phase_x,Phase_y,Phase_z,Nx,Ny,Nz);
	pmmc_MeshGradient(SignDist,Sx,Sy,Sz,Nx,Ny,Nz);
	pmmc_MeshCurvature(Phase, MeanCurvature, GaussCurvature, Nx, Ny, Nz);
	
	/// Compute volume averages
	for (k=0; k<Nz; k++){
		for (j=0; j<Ny; j++){
			for (i=0; i<Nx; i++){
				if ( SignDist(i,j,k) > 0.0 ){

					// 1-D index for this cube corner
					n = i + j*Nx + k*Nx*Ny;
					// Compute the non-wetting phase volume contribution
					if ( Phase(i,j,k) > 0.0 )
						nwp_volume += 1.0;

					// volume averages over the non-wetting phase
					if ( Phase(i,j,k) > 0.99 ){
						// volume the excludes the interfacial region
						vol_n += 1.0;
						// pressure
						pan += Press(i,j,k);
						// velocity
						van(0) += Vel_x(i,j,k);
						van(1) += Vel_y(i,j,k);
						van(2) += Vel_z(i,j,k);
					}

					// volume averages over the wetting phase
					if ( Phase(i,j,k) < -0.99 ){
						// volume the excludes the interfacial region
						vol_w += 1.0;
						// pressure
						paw += Press(i,j,k);
						// velocity
						vaw(0) += Vel_x(i,j,k);
						vaw(1) += Vel_y(i,j,k);
						vaw(2) += Vel_z(i,j,k);
					}
				}
			}
		}
	}
	
	printf("vol_n = %f \n",vol_n);
	printf("vol_w = %f \n",vol_w);
	
	// End of the loop to set the values
	awn = aws = ans = lwns = 0.0;
	nwp_volume = 0.0;
	As = 0.0;
	// Compute phase averages
	pan = paw = 0.0;
	vaw(0) = vaw(1) = vaw(2) = 0.0;
	Gwn(0) = Gwn(1) = Gwn(2) = Gwn(3) = Gwn(4) = Gwn(5) = 0.0;
	Gws(0) = Gws(1) = Gws(2) = Gws(3) = Gws(4) = Gws(5) = 0.0;
	Gns(0) = Gns(1) = Gns(2) = Gns(3) = Gns(4) = Gns(5) = 0.0;

	vol_w = vol_n =0.0;
	
	FILE *WN_TRIS;
	WN_TRIS = fopen("wn-tris.out","w");
	
	FILE *NS_TRIS;
	NS_TRIS = fopen("ns-tris.out","w");
	
	FILE *WS_TRIS;
	WS_TRIS = fopen("ws-tris.out","w");
	
	FILE *WNS_PTS;
	WNS_PTS = fopen("wns-pts.out","w");
	
	printf("ncubes = %i,\n",ncubes);
	
	for (c=0;c<ncubes;c++){
		// Get cube from the list
		i = cubeList(0,c);
		j = cubeList(1,c);
		k = cubeList(2,c);
		
		// Run PMMC
		n_local_sol_tris = 0;
		n_local_sol_pts = 0;
		n_local_nws_pts = 0;
		n_nw_pts=0,n_ns_pts=0,n_ws_pts=0,n_nws_pts=0, map=0;
		n_nw_tris=0, n_ns_tris=0, n_ws_tris=0, n_nws_seg=0;

		// Construct the interfaces and common curve
		pmmc_ConstructLocalCube(SignDist, Phase, solid_isovalue, fluid_isovalue,
				nw_pts, nw_tris, values, ns_pts, ns_tris, ws_pts, ws_tris,
				local_nws_pts, nws_pts, nws_seg, local_sol_pts, local_sol_tris,
				n_local_sol_tris, n_local_sol_pts, n_nw_pts, n_nw_tris,
				n_ws_pts, n_ws_tris, n_ns_tris, n_ns_pts, n_local_nws_pts, n_nws_pts, n_nws_seg,
				i, j, k, Nx, Ny, Nz);
		
		// Compute the velocity of the wn interface
		pmmc_InterfaceSpeed(dPdt, Phase_x, Phase_y, Phase_z, CubeValues, nw_pts, nw_tris,
							NormalVector, InterfaceSpeed, vawn, i, j, k, n_nw_pts, n_nw_tris);
		
//		pmmc_InterfaceSpeed(dPdt, Phase_x, Phase_y, Phase_z, CubeValues, nw_pts, nw_tris,
//							NormalVector, InterfaceSpeed, vawn, i, j, k, n_nw_pts, n_nw_tris);
		
		
		// Compute the average contact angle
		efawns += pmmc_CubeContactAngle(CubeValues,ContactAngle,Phase_x,Phase_y,Phase_z,Sx,Sy,Sz,
										local_nws_pts,i,j,k,n_local_nws_pts);
		
	//	printf("efawns= %f \n", efawns);
		if (isnan(efawns))
			c = ncubes;
		
		// Compute the curvature of the wn interface
		Jwn += pmmc_CubeSurfaceInterpValue(CubeValues, MeanCurvature, nw_pts, nw_tris,
											Curvature, i, j, k, n_nw_pts, n_nw_tris);
		
		// Compute the surface orientation and the interfacial area
		awn += pmmc_CubeSurfaceOrientation(Gwn,nw_pts,nw_tris,n_nw_tris);
		ans += pmmc_CubeSurfaceOrientation(Gns,ns_pts,ns_tris,n_ns_tris);
		aws += pmmc_CubeSurfaceOrientation(Gws,ws_pts,ws_tris,n_ws_tris);
		
		//*******************************************************************
		// Compute the Interfacial Areas, Common Line length
//		awn += pmmc_CubeSurfaceArea(nw_pts,nw_tris,n_nw_tris);
//		ans += pmmc_CubeSurfaceArea(ns_pts,ns_tris,n_ns_tris);
//		aws += pmmc_CubeSurfaceArea(ws_pts,ws_tris,n_ws_tris);
		As += pmmc_CubeSurfaceArea(local_sol_pts,local_sol_tris,n_local_sol_tris);
		lwns +=  pmmc_CubeCurveLength(local_nws_pts,n_local_nws_pts);
		
		//.......................................................................................
		// Write the triangle lists to text file
		for (r=0;r<n_nw_tris;r++){
			A = nw_pts(nw_tris(0,r));
			B = nw_pts(nw_tris(1,r));
			C = nw_pts(nw_tris(2,r));
			fprintf(WN_TRIS,"%f %f %f %f %f %f %f %f %f \n",A.x,A.y,A.z,B.x,B.y,B.z,C.x,C.y,C.z);
		}		
		for (r=0;r<n_ws_tris;r++){
			A = ws_pts(ws_tris(0,r));
			B = ws_pts(ws_tris(1,r));
			C = ws_pts(ws_tris(2,r));
			fprintf(WS_TRIS,"%f %f %f %f %f %f %f %f %f \n",A.x,A.y,A.z,B.x,B.y,B.z,C.x,C.y,C.z);
		}
		for (r=0;r<n_ns_tris;r++){
			A = ns_pts(ns_tris(0,r));
			B = ns_pts(ns_tris(1,r));
			C = ns_pts(ns_tris(2,r));
			fprintf(NS_TRIS,"%f %f %f %f %f %f %f %f %f \n",A.x,A.y,A.z,B.x,B.y,B.z,C.x,C.y,C.z);
		}
		for (p=0; p < n_nws_pts; p++){
			P = nws_pts(p);
			fprintf(WNS_PTS,"%f %f %f \n",P.x, P.y, P.z);
		}
		
	}
	fclose(WN_TRIS);
	fclose(NS_TRIS);
	fclose(WS_TRIS);
	fclose(WNS_PTS);

	printf("Jwn = %f \n",Jwn);
	printf("awn = %f \n",awn);
	printf("efawns = %f \n",efawns);
	printf("lwns = %f \n",lwns);
	printf("efawns = %f \n",efawns/lwns);

	Jwn /= awn;
	efawns /= lwns;
	for (i=0; i<3; i++)		vawn(i) /= awn;
	for (i=0; i<6; i++)		Gwn(i) /= awn;
	for (i=0; i<6; i++)		Gns(i) /= ans;
	for (i=0; i<6; i++)		Gws(i) /= aws;
	
	awn = awn*iVol;
	aws = aws*iVol;
	ans = ans*iVol;
	lwns = lwns*iVol;

	printf("--------------------------------------------------------------------------------------\n");
	printf("sw pw pn vw[x, y, z] vn[x, y, z] ");			// Volume averages
	printf("awn ans aws Jwn vwn[x, y, z] lwns efawns ");	// Interface and common curve averages
	printf("Gwn [xx, yy, zz, xy, xz, yz] ");				// Orientation tensors
	printf("Gws [xx, yy, zz, xy, xz, yz] ");
	printf("Gns [xx, yy, zz, xy, xz, yz] \n");
	printf("--------------------------------------------------------------------------------------\n");
	printf("%.5g %.5g %.5g ",sw,paw,pan);					// saturation and pressure
	printf("%.5g %.5g %.5g ",vaw(0),vaw(1),vaw(2));			// average velocity of w phase
	printf("%.5g %.5g %.5g ",van(0),van(1),van(2));			// average velocity of n phase
	printf("%.5g %.5g %.5g ",awn,ans,aws);					// interfacial areas
	printf("%.5g ",Jwn);									// curvature of wn interface
	printf("%.5g ", lwns);									// common curve length
	printf("%.5g ",efawns);									// average contact angle
	printf("%.5g %.5g %.5g %.5g %.5g %.5g ",
			Gwn(0),Gwn(1),Gwn(2),Gwn(3),Gwn(4),Gwn(5));		// orientation of wn interface
	printf("%.5g %.5g %.5g %.5g %.5g %.5g ",
			Gns(0),Gns(1),Gns(2),Gns(3),Gns(4),Gns(5));		// orientation of ns interface	
	printf("%.5g %.5g %.5g %.5g %.5g %.5g \n",
			Gws(0),Gws(1),Gws(2),Gws(3),Gws(4),Gws(5));		// orientation of ws interface


	/*	printf("-------------------------------- \n");
	printf("NWP volume = %f \n", nwp_volume);
	printf("Area wn = %f, Analytical = %f \n", awn,2*PI*RADIUS*RADIUS);
	printf("Area ns = %f, Analytical = %f \n", ans, 2*PI*RADIUS*(N-2)-4*PI*RADIUS*HEIGHT);
	printf("Area ws = %f, Analytical = %f \n", aws, 4*PI*RADIUS*HEIGHT);
	printf("Area s = %f, Analytical = %f \n", As, 2*PI*RADIUS*(N-2));
	printf("Length wns = %f, Analytical = %f \n", lwns, 4*PI*RADIUS);
//	printf("Cos(theta_wns) = %f,  Analytical = %f \n",efawns/lwns,1.0*RADIUS/CAPRAD);
	printf("Interface Velocity = %f,%f,%f \n",vawn(0)/awn,vawn(1)/awn,vawn(2)/awn);
	printf("-------------------------------- \n");	
	//.........................................................................	
*/	
	FILE *PHASE;
	PHASE = fopen("Phase.out","wb");
	fwrite(Phase.data,8,N,PHASE);
	fclose(PHASE);
	
	FILE *SOLID;
	SOLID = fopen("Distance.out","wb");
	fwrite(SignDist.data,8,N,SOLID);
	fclose(SOLID);

}
