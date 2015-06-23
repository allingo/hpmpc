/**************************************************************************************************
*                                                                                                 *
* This file is part of HPMPC.                                                                     *
*                                                                                                 *
* HPMPC -- Library for High-Performance implementation of solvers for MPC.                        *
* Copyright (C) 2014-2015 by Technical University of Denmark. All rights reserved.                *
*                                                                                                 *
* HPMPC is free software; you can redistribute it and/or                                          *
* modify it under the terms of the GNU Lesser General Public                                      *
* License as published by the Free Software Foundation; either                                    *
* version 2.1 of the License, or (at your option) any later version.                              *
*                                                                                                 *
* HPMPC is distributed in the hope that it will be useful,                                        *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                                            *
* See the GNU Lesser General Public License for more details.                                     *
*                                                                                                 *
* You should have received a copy of the GNU Lesser General Public                                *
* License along with HPMPC; if not, write to the Free Software                                    *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                  *
*                                                                                                 *
* Author: Gianluca Frison, giaf (at) dtu.dk                                                       *
*                                                                                                 *
**************************************************************************************************/

#include <math.h>

#include "../include/aux_d.h"
#include "../include/blas_d.h"
#include "../include/lqcp_aux.h"
#include "../include/block_size.h"



void d_cond_Q(int N, int nx, int nu, double **pA, int diag_Q, int nzero_Q_N, double **pQ, double **pL, int compute_Gamma_0, double **pGamma_0, double **pGamma_0_Q, double *pH_Q, double *work)
	{

	const int bs = D_MR;
	const int ncl = D_NCL;

	int cnx = (nx+ncl-1)/ncl*ncl;
	//int cNnx = (N*nx+ncl-1)/ncl*ncl;

	int ii, jj;

	int N1 = N;
	if(nzero_Q_N==0)
		N1 = N-1;
	
	if(compute_Gamma_0)
		{
		dgetr_lib(nx, nx, 0, pA[0], cnx, 0, pGamma_0[0], cnx);
		for(ii=1; ii<N; ii++)
			{
			dgemm_nt_lib(nx, nx, nx, pGamma_0[ii-1], cnx, pA[ii], cnx, pGamma_0[ii], cnx, pGamma_0[ii], cnx, 0, 0, 0);
			}
		}
	
	if(diag_Q)
		{

#if 0
		for(jj=0; jj<nx; jj++) pL[1][jj] = sqrt(pQ[1][jj]);
		dgemm_diag_right_lib(nx, nx, pGamma_0[0], cnx, pL[1], pGamma_0_Q[0], cnx, pGamma_0_Q[0], cnx, 0);
		dsyrk_nt_lib(nx, nx, nx, pGamma_0_Q[0], cnx, pGamma_0_Q[0], cnx, pH_Q, cnx, pH_Q, cnx, 0);
		for(ii=1; ii<N1; ii++)
			{
			for(jj=0; jj<nx; jj++) pL[ii+1][jj] = sqrt(pQ[ii+1][jj]);
			dgemm_diag_right_lib(nx, nx, pGamma_0[ii], cnx, pL[ii+1], pGamma_0_Q[ii], cnx, pGamma_0_Q[ii], cnx, 0);
			dsyrk_nt_lib(nx, nx, nx, pGamma_0_Q[ii], cnx, pGamma_0_Q[ii], cnx, pH_Q, cnx, pH_Q, cnx, 1);
			}
		d_add_diag_pmat(nx, pH_Q, cnx, pQ[0]);
#else
		if(N1>0)
			{
			dgemm_diag_right_lib(nx, nx, pGamma_0[0], cnx, pQ[1], pGamma_0_Q[0], cnx, pGamma_0_Q[0], cnx, 0);
			dsyrk_nt_lib(nx, nx, nx, pGamma_0_Q[0], cnx, pGamma_0[0], cnx, pH_Q, cnx, pH_Q, cnx, 0);
			}
		else
			{
			d_set_pmat(nx, nx, 0.0, 0, pH_Q, cnx);
			}
		for(ii=1; ii<N1; ii++)
			{
			dgemm_diag_right_lib(nx, nx, pGamma_0[ii], cnx, pQ[ii+1], pGamma_0_Q[ii], cnx, pGamma_0_Q[ii], cnx, 0);
			dsyrk_nt_lib(nx, nx, nx, pGamma_0_Q[ii], cnx, pGamma_0[ii], cnx, pH_Q, cnx, pH_Q, cnx, 1);
			}
		d_add_diag_pmat(nx, pH_Q, cnx, pQ[0]);
#endif

		}
	else
		{
#if 1

		dpotrf_lib(nx, nx, pQ[1], cnx, pL[1], cnx, work);
		dtrtr_l_lib(nx, 0, pL[1], cnx, pL[1], cnx);
		dtrmm_l_lib(nx, nx, pGamma_0[0], cnx, pL[1], cnx, pGamma_0_Q[0], cnx);
		dsyrk_nt_lib(nx, nx, nx, pGamma_0_Q[0], cnx, pGamma_0_Q[0], cnx, pQ[0], cnx, pH_Q, cnx, 1);
		for(ii=1; ii<N1; ii++)
			{
			dpotrf_lib(nx, nx, pQ[ii+1], cnx, pL[ii+1], cnx, work);
			dtrtr_l_lib(nx, 0, pL[ii+1], cnx, pL[ii+1], cnx);
			dtrmm_l_lib(nx, nx, pGamma_0[ii], cnx, pL[ii+1], cnx, pGamma_0_Q[ii], cnx);
			dsyrk_nt_lib(nx, nx, nx, pGamma_0_Q[ii], cnx, pGamma_0_Q[ii], cnx, pH_Q, cnx, pH_Q, cnx, 1);
			}
#else
	
		// Gamma_0 * bar_Q * Gamma_0'
		dgemm_nt_lib(nx, nx, nx, pGamma_0[0], cnx, pQ[1], cnx, pGamma_0_Q[0], cnx, pGamma_0_Q[0], cnx, 0, 0, 0);
		//dgemm_nt_lib(nx, nx, nx, pGamma_0_Q[0], cnx, pGamma_0[0], cnx, pQ[0], cnx, pH_Q, cnx, 1, 0, 0);
		dsyrk_nt_lib(nx, nx, nx, pGamma_0_Q[0], cnx, pGamma_0[0], cnx, pQ[0], cnx, pH_Q, cnx, 1);
		for(ii=1; ii<N1; ii++)
			{
			dgemm_nt_lib(nx, nx, nx, pGamma_0[ii], cnx, pQ[ii+1], cnx, pGamma_0_Q[ii], cnx, pGamma_0_Q[ii], cnx, 0, 0, 0);
			//dgemm_nt_lib(nx, nx, nx, pGamma_0_Q[ii], cnx, pGamma_0[ii], cnx, pH_Q, cnx, pH_Q, cnx, 1, 0, 0);
			dsyrk_nt_lib(nx, nx, nx, pGamma_0_Q[ii], cnx, pGamma_0[ii], cnx, pH_Q, cnx, pH_Q, cnx, 1);
			}
#endif
		}

	}



void d_cond_R(int N, int nx, int nu, int N2_cond, double **pA, double **pAt, double **pBt, int diag_Q, int nzero_Q_N, double **pQ, double **pL, int nzero_S, double **pS, double **pR, int compute_Gamma_u, double **pGamma_u, double **pGamma_u_Q, double **pGamma_u_Q_A, double *pH_R)
	{

	const int bs = D_MR;
	const int ncl = D_NCL;

	int cnx = (nx+ncl-1)/ncl*ncl;
	int cnu = (nu+ncl-1)/ncl*ncl;
	int cNnu = (N*nu+ncl-1)/ncl*ncl;
	//int cNnx = (N*nx+ncl-1)/ncl*ncl;

	int ii, jj, offset, i_temp;

	int N1 = N;
	if(nzero_Q_N==0)
		N1 = N-1;
	
	// Gamma_u
	if(compute_Gamma_u)
		{
		dgecp_lib(nu, nx, 0, pBt[0], cnx, 0, pGamma_u[0], cnx);
		for(ii=1; ii<N; ii++)
			{
			offset = ii*nu;
#if defined(TARGET_X64_AVX2) || defined(TARGET_X64_AVX) || defined(TARGET_C99_4X4)
			dgemm_nt_lib(nx, ii*nu, nx, pA[ii], cnx, pGamma_u[ii-1], cnx, pGamma_u[ii], cnx, pGamma_u[ii], cnx, 0, 0, 1); // (A * Gamma_u^T)^T
#else
			dgemm_nt_lib(ii*nu, nx, nx, pGamma_u[ii-1], cnx, pA[ii], cnx, pGamma_u[ii], cnx, pGamma_u[ii], cnx, 0, 0, 0); // Gamma_u * A^T
#endif
			dgecp_lib(nu, nx, 0, pBt[ii], cnx, offset, pGamma_u[ii]+offset/bs*bs*cnx+offset%bs, cnx);
			}
		}
		
	if(N2_cond)
		{

		// Gamma_u * Q
		if(diag_Q)
			{
			for(ii=0; ii<N1; ii++)
				{
				dgemm_diag_right_lib((ii+1)*nu, nx, pGamma_u[ii], cnx, pQ[ii+1], pGamma_u_Q[ii], cnx, pGamma_u_Q[ii], cnx, 0);
				}
			}
		else
			{
			for(ii=0; ii<N1; ii++)
				{
#if defined(TARGET_X64_AVX2) || defined(TARGET_X64_AVX) || defined(TARGET_C99_4X4)
				dgemm_nt_lib(nx, (ii+1)*nu, nx, pQ[ii+1], cnx, pGamma_u[ii], cnx, pGamma_u_Q[ii], cnx, pGamma_u_Q[ii], cnx, 0, 0, 1); // (A * Gamma_u^T)^T
#else
				dgemm_nt_lib((ii+1)*nu, nx, nx, pGamma_u[ii], cnx, pQ[ii+1], cnx, pGamma_u_Q[ii], cnx, pGamma_u_Q[ii], cnx, 0, 0, 0); // Gamma_u * A^T
#endif
				}
			}

		if(nzero_Q_N==0)
			d_set_pmat(N*nu, nx, 0.0, 0, pGamma_u_Q[N-1], cnx);

		if(nzero_Q_N==0)
			d_set_pmat(N*nu, nx, 0.0, 0, pGamma_u_Q_A[N-1], cnx);
		
		// Gamma_u_Q * bar_A
		dgecp_lib(N1*nu, nx, 0, pGamma_u_Q[N1-1], cnx, 0, pGamma_u_Q_A[N1-1], cnx);
		for(ii=N1-1; ii>0; ii--)
			{
#if defined(TARGET_X64_AVX2) || defined(TARGET_X64_AVX) || defined(TARGET_C99_4X4)
			dgemm_nt_lib(nx, ii*nu, nx, pAt[ii], cnx, pGamma_u_Q_A[ii], cnx, pGamma_u_Q[ii-1], cnx, pGamma_u_Q_A[ii-1], cnx, 1, 1, 1);
#else
			dgemm_nt_lib(ii*nu, nx, nx, pGamma_u_Q_A[ii], cnx, pAt[ii], cnx, pGamma_u_Q[ii-1], cnx, pGamma_u_Q_A[ii-1], cnx, 1, 0, 0);
#endif
			}

		if(nzero_S)
			{
			// Gamma_u * bar_S
			for(ii=1; ii<N; ii++)
				{
				dgemm_nt_lib(ii*nu, nu, nx, pGamma_u[ii-1], cnx, pS[ii], cnx, pH_R+ii*nu*bs, cNnu, pH_R+ii*nu*bs, cNnu, 0, 0, 0);
				}
			}
		else // TODO use d_set_pmat
			{
			for(ii=0; ii<N*nu; ii+=4)
				{
				for(jj=ii; jj<N*nu; jj++)
					{
					pH_R[ii*cNnu+jj*bs+0] = 0.0;
					pH_R[ii*cNnu+jj*bs+1] = 0.0;
					pH_R[ii*cNnu+jj*bs+2] = 0.0;
					pH_R[ii*cNnu+jj*bs+3] = 0.0;
					}
				}
			}
		
		// R
		for(ii=0; ii<N; ii++)
			{
			dgecp_lib(nu, nu, 0, pR[ii], cnu, ii*nu, pH_R+(ii*nu)/bs*bs*cNnu+(ii*nu)%bs+ii*nu*bs, cNnu);
			}

		// Gamma_u_Q_A * B
		for(ii=0; ii<N1; ii++)
			{
			dgemm_nt_lib((ii+1)*nu, nu, nx, pGamma_u_Q_A[ii], cnx, pBt[ii], cnx, pH_R+ii*nu*bs, cNnu, pH_R+ii*nu*bs, cNnu, 1, 0, 0);
			}

		// transpose H in the lower triangular
		dtrtr_u_lib(N*nu, pH_R, cNnu, pH_R, cNnu);

		}
	else // N3 cond
		{
		
		// Gamma_u * Q
		if(diag_Q)
			{
			for(ii=0; ii<N1; ii++)
				{
				dgemm_diag_right_lib((ii+1)*nu, nx, pGamma_u[ii], cnx, pQ[ii+1], pGamma_u_Q[ii], cnx, pGamma_u_Q[ii], cnx, 0);
				}
			}
		else
			{
			for(ii=0; ii<N1; ii++)
				{
#if 1
				dtrmm_l_lib((ii+1)*nu, nx, pGamma_u[ii], cnx, pL[ii+1], cnx, pGamma_u_Q[ii], cnx);
#else
#if defined(TARGET_X64_AVX2) || defined(TARGET_X64_AVX) || defined(TARGET_C99_4X4)
				dgemm_nt_lib(nx, (ii+1)*nu, nx, pQ[ii+1], cnx, pGamma_u[ii], cnx, pGamma_u_Q[ii], cnx, pGamma_u_Q[ii], cnx, 0, 0, 1); // (A * Gamma_u^T)^T
#else
				dgemm_nt_lib((ii+1)*nu, nx, nx, pGamma_u[ii], cnx, pQ[ii+1], cnx, pGamma_u_Q[ii], cnx, pGamma_u_Q[ii], cnx, 0, 0, 0); // Gamma_u * A^T
#endif
#endif
				}
			}
		if(nzero_Q_N==0)
			d_set_pmat(N*nu, nx, 0.0, 0, pGamma_u_Q[N-1], cnx);
		

		if(nzero_S)
			{
			// Gamma_u * bar_S
			for(ii=1; ii<N; ii++)
				{
				dgemm_nt_lib(ii*nu, nu, nx, pGamma_u[ii-1], cnx, pS[ii], cnx, pH_R+ii*nu*bs, cNnu, pH_R+ii*nu*bs, cNnu, 0, 0, 0);
				}

			// transpose H in the lower triangular
			dtrtr_u_lib(N*nu, pH_R, cNnu, pH_R, cNnu);
			}
		else // TODO use d_set_pmat
			{
			for(ii=0; ii<N*nu; ii+=4)
				{
				i_temp = ii+4<cNnu ? ii+4 : cNnu;
				for(jj=0; jj<i_temp; jj++)
					{
					pH_R[ii*cNnu+jj*bs+0] = 0.0;
					pH_R[ii*cNnu+jj*bs+1] = 0.0;
					pH_R[ii*cNnu+jj*bs+2] = 0.0;
					pH_R[ii*cNnu+jj*bs+3] = 0.0;
					}
				}
			}
			
		// R
		for(ii=0; ii<N; ii++)
			{
			dgecp_lib(nu, nu, 0, pR[ii], cnu, ii*nu, pH_R+(ii*nu)/bs*bs*cNnu+(ii*nu)%bs+ii*nu*bs, cNnu);
			}

		if(diag_Q)
			{
			for(ii=0; ii<N1; ii++)
				dsyrk_nt_lib((N1-ii)*nu, (N1-ii)*nu, nx, pGamma_u_Q[N1-1-ii], cnx, pGamma_u[N1-1-ii], cnx, pH_R, cNnu, pH_R, cNnu, 1); 
			}
		else
			{
			for(ii=0; ii<N1; ii++)
				dsyrk_nt_lib((N1-ii)*nu, (N1-ii)*nu, nx, pGamma_u_Q[N1-1-ii], cnx, pGamma_u_Q[N1-1-ii], cnx, pH_R, cNnu, pH_R, cNnu, 1); 
			}

		}
	
	}



void d_cond_St(int N, int nx, int nu, int nzero_S, double **pS, int nzero_Q_N, double **pGamma_0, int use_pGamma_0_Q, double **pGamma_0_Q, double **pGamma_u_Q, double *pH_St)
	{

	const int bs = D_MR;
	const int ncl = D_NCL;

	int cnx = (nx+ncl-1)/ncl*ncl;
	int cnu = (nu+ncl-1)/ncl*ncl;
	int cNnu = (N*nu+ncl-1)/ncl*ncl;
	//int cNnx = (N*nx+ncl-1)/ncl*ncl;

	int ii;

	int N1 = N;
	if(nzero_Q_N==0)
		N1 = N-1;
	
	if(nzero_S)
		{
		// Gamma_0 * bar_S
		dgetr_lib(nu, nx, 0, pS[0], cnx, 0, pH_St, cNnu);
		for(ii=1; ii<N; ii++)
			{
			dgemm_nt_lib(nx, nu, nx, pGamma_0[ii-1], cnx, pS[ii], cnx, pH_St+ii*nu*bs, cNnu, pH_St+ii*nu*bs, cNnu, 0, 0, 0);
			}

		if(use_pGamma_0_Q)
			{
			for(ii=0; ii<N1; ii++)
				{
				dgemm_nt_lib(nx, (ii+1)*nu, nx, pGamma_0_Q[ii], cnx, pGamma_u_Q[ii], cnx, pH_St, cNnu, pH_St, cNnu, 1, 0, 0);
				}
			}
		else
			{
			for(ii=0; ii<N1; ii++)
				{
				dgemm_nt_lib(nx, (ii+1)*nu, nx, pGamma_0[ii], cnx, pGamma_u_Q[ii], cnx, pH_St, cNnu, pH_St, cNnu, 1, 0, 0);
				}
			}
		}
	else
		{
		if(use_pGamma_0_Q)
			{
			dgemm_nt_lib(nx, N1*nu, nx, pGamma_0_Q[N1-1], cnx, pGamma_u_Q[N1-1], cnx, pH_St, cNnu, pH_St, cNnu, 0, 0, 0);
			if(nzero_Q_N==0)
				d_set_pmat(nx, nu, 0.0, 0, pH_St+N1*nu*bs, cNnu);
			for(ii=0; ii<N1-1; ii++)
				{
				dgemm_nt_lib(nx, (ii+1)*nu, nx, pGamma_0_Q[ii], cnx, pGamma_u_Q[ii], cnx, pH_St, cNnu, pH_St, cNnu, 1, 0, 0);
				}
			}
		else
			{
			dgemm_nt_lib(nx, N1*nu, nx, pGamma_0[N1-1], cnx, pGamma_u_Q[N1-1], cnx, pH_St, cNnu, pH_St, cNnu, 0, 0, 0);
			if(nzero_Q_N==0)
				d_set_pmat(nx, nu, 0.0, 0, pH_St+N1*nu*bs, cNnu);
			for(ii=0; ii<N1-1; ii++)
				{
				dgemm_nt_lib(nx, (ii+1)*nu, nx, pGamma_0[ii], cnx, pGamma_u_Q[ii], cnx, pH_St, cNnu, pH_St, cNnu, 1, 0, 0);
				}
			}
		}
	
	}



void d_cond_q(int N, int nx, int nu, double **pA, double **b, int diag_Q, int nzero_Q_N, double **pQ, double **q, double **pGamma_0, int compute_Gamma_b, double **Gamma_b, int compute_Gamma_b_q, double **Gamma_b_q, double *H_q)
	{

	const int bs = D_MR;
	const int ncl = D_NCL;

	int cnx = (nx+ncl-1)/ncl*ncl;
	int cnu = (nu+ncl-1)/ncl*ncl;
	int cNnu = (N*nu+ncl-1)/ncl*ncl;
	//int cNnx = (N*nx+ncl-1)/ncl*ncl;

	int ii;

	int N1 = N;
	if(nzero_Q_N==0)
		N1 = N-1;
	
	// Gamma_b
	if(compute_Gamma_b)
		{
		d_copy_mat(nx, 1, b[0], 1, Gamma_b[0], 1);
		for(ii=1; ii<N; ii++)
			{
			dgemv_n_lib(nx, nx, pA[ii], cnx, Gamma_b[ii-1], b[ii], Gamma_b[ii], 1);
			}
		}
	
	// Gamma_b * Q + q
	if(compute_Gamma_b_q)
		{
		if(diag_Q)
			{
			for(ii=0; ii<N1; ii++)
				{
				dgemv_diag_lib(nx, pQ[ii+1], Gamma_b[ii], q[ii+1], Gamma_b_q[ii], 1);
				}
			}
		else
			{
			for(ii=0; ii<N1; ii++)
				{
				//dgemv_n_lib(nx, nx, pQ[ii+1], cnx, Gamma_b[ii], q[ii+1], Gamma_b_q[ii], 1);
				dsymv_lib(nx, nx, pQ[ii+1], cnx, Gamma_b[ii], q[ii+1], Gamma_b_q[ii], 1);
				}
			}
		}
		
	// Gamma_0' * Gamma_b_q
	d_copy_mat(nx, 1, q[0], 1, H_q, 1);
	for(ii=0; ii<N1; ii++)
		{
		dgemv_n_lib(nx, nx, pGamma_0[ii], cnx, Gamma_b_q[ii], H_q, H_q, 1);
		}
	
	}



void d_cond_r(int N, int nx, int nu, double **pA, double **b, int diag_Q, int nzero_Q_N, double **pQ, int nzero_S, double **pS, double **q, double **r, double **pGamma_u, int compute_Gamma_b, double **Gamma_b, int compute_Gamma_b_q, double **Gamma_b_q, double *H_r)
	{

	const int bs = D_MR;
	const int ncl = D_NCL;

	int cnx = (nx+ncl-1)/ncl*ncl;
	//int cNnx = (N*nx+ncl-1)/ncl*ncl;

	int ii;

	int N1 = N;
	if(nzero_Q_N==0)
		N1 = N-1;
	
	// Gamma_b
	if(compute_Gamma_b)
		{
		d_copy_mat(nx, 1, b[0], 1, Gamma_b[0], 1);
		for(ii=1; ii<N; ii++)
			{
			dgemv_n_lib(nx, nx, pA[ii], cnx, Gamma_b[ii-1], b[ii], Gamma_b[ii], 1);
			}
		}

	// barS * Gamma_b
	if(nzero_S)
		{
		for(ii=0; ii<N; ii++)
			{
			dgemv_n_lib(nu, nx, pS[ii], cnx, Gamma_b[ii], r[ii], H_r+ii*nu, 1);
			}
		}
	else
		{
		for(ii=0; ii<N; ii++)
			{
			d_copy_mat(nu, 1, r[ii], 1, H_r+ii*nu, 1);
			}
		}
	
	// Gamma_b * Q + q
	if(compute_Gamma_b_q)
		{
		if(diag_Q)
			{
			for(ii=0; ii<N1; ii++)
				{
				dgemv_diag_lib(nx, pQ[ii+1], Gamma_b[ii], q[ii+1], Gamma_b_q[ii], 1);
				}
			}
		else
			{
			for(ii=0; ii<N1; ii++)
				{
				//dgemv_n_lib(nx, nx, pQ[ii+1], cnx, Gamma_b[ii], q[ii+1], Gamma_b_q[ii], 1);
				dsymv_lib(nx, nx, pQ[ii+1], cnx, Gamma_b[ii], q[ii+1], Gamma_b_q[ii], 1);
				}
			}
		}

	// Gamma_u * Gamma_b_q
	for(ii=0; ii<N1; ii++)
		{
		dgemv_n_lib((ii+1)*nu, nx, pGamma_u[ii], cnx, Gamma_b_q[ii], H_r, H_r, 1);
		}
		
	}


void d_cond_A(int N, int nx, int nu, double **pA, int compute_Gamma_0, double **pGamma_0, double *pH_A)
	{

	const int bs = D_MR;
	const int ncl = D_NCL;

	int cnx = (nx+ncl-1)/ncl*ncl;
	//int cNnx = (N*nx+ncl-1)/ncl*ncl;

	int ii, jj;

	if(compute_Gamma_0)
		{
		dgetr_lib(nx, nx, 0, pA[0], cnx, 0, pGamma_0[0], cnx);
		for(ii=1; ii<N; ii++)
			{
			dgemm_nt_lib(nx, nx, nx, pGamma_0[ii-1], cnx, pA[ii], cnx, pGamma_0[ii], cnx, pGamma_0[ii], cnx, 0, 0, 0);
			}
		}
	
	dgetr_lib(nx, nx, 0, pGamma_0[N-1], cnx, 0, pH_A, cnx);

	}



void d_cond_B(int N, int nx, int nu, double **pA, double **pBt, int compute_Gamma_u, double **pGamma_u, double *pH_B)
	{
	
	const int bs = D_MR;
	const int ncl = D_NCL;

	int cnx = (nx+ncl-1)/ncl*ncl;
	int cnu = (nu+ncl-1)/ncl*ncl;
	int cNnu = (N*nu+ncl-1)/ncl*ncl;
	//int cNnx = (N*nx+ncl-1)/ncl*ncl;

	int ii, jj, offset, i_temp;

	// Gamma_u
	if(compute_Gamma_u)
		{
		dgecp_lib(nu, nx, 0, pBt[0], cnx, 0, pGamma_u[0], cnx);
		for(ii=1; ii<N; ii++)
			{
			offset = ii*nu;
#if defined(TARGET_X64_AVX2) || defined(TARGET_X64_AVX) || defined(TARGET_C99_4X4)
			dgemm_nt_lib(nx, ii*nu, nx, pA[ii], cnx, pGamma_u[ii-1], cnx, pGamma_u[ii], cnx, pGamma_u[ii], cnx, 0, 0, 1); // (A * Gamma_u^T)^T
#else
			dgemm_nt_lib(ii*nu, nx, nx, pGamma_u[ii-1], cnx, pA[ii], cnx, pGamma_u[ii], cnx, pGamma_u[ii], cnx, 0, 0, 0); // Gamma_u * A^T
#endif
			dgecp_lib(nu, nx, 0, pBt[ii], cnx, offset, pGamma_u[ii]+offset/bs*bs*cnx+offset%bs, cnx);
			}
		}
	
	dgetr_lib(N*nu, nx, 0, pGamma_u[N-1], cnx, 0, pH_B, cNnu);

	}



void d_cond_b(int N, int nx, int nu, double **pA, double **b, int compute_Gamma_b, double **Gamma_b, double *H_b)
	{

	const int bs = D_MR;
	const int ncl = D_NCL;

	int cnx = (nx+ncl-1)/ncl*ncl;
	int cnu = (nu+ncl-1)/ncl*ncl;
	int cNnu = (N*nu+ncl-1)/ncl*ncl;
	//int cNnx = (N*nx+ncl-1)/ncl*ncl;

	int ii;

	// Gamma_b
	if(compute_Gamma_b)
		{
		d_copy_mat(nx, 1, b[0], 1, Gamma_b[0], 1);
		for(ii=1; ii<N; ii++)
			{
			dgemv_n_lib(nx, nx, pA[ii], cnx, Gamma_b[ii-1], b[ii], Gamma_b[ii], 1);
			}
		}
	
	d_copy_mat(nx, 1, Gamma_b[N-1], 1, H_b, 1);
	
	}
	


#if 1
int d_cond_lqcp_work_space(int N, int nx, int nu, int N2)
	{

	const int bs = D_MR;
	const int ncl = D_NCL;

	int pnx = (nx+bs-1)/bs*bs;
	int cnx = (nx+ncl-1)/ncl*ncl;

	// find problem size
	int N1 = N/N2; // floor
	int R1 = N - N2*N1; // the first r1 stages are of size N1+1
	int M1 = R1>0 ? N1+1 : N1;

	int work_space_size = 0;

	int jj;

	for(jj=0; jj<M1; jj++)
		{
		work_space_size += 3*pnx*cnx + 3*((jj+1)*nu+bs-1)/bs*bs*cnx + 2*pnx;
		}
	work_space_size += pnx*cnx + pnx;

	return work_space_size;

	}



void d_cond_lqcp(int N, int nx, int nu, double **hpA, double **hpAt, double **hpBt, double **hb, double **hpR, int nzero_S, double **hpS, int diag_Q, double **hpQ, double **hr, double **hq, int N2, int *nx2, int *nu2, double **hpA2, double **hpB2, double **hb2, double **hpR2, double **hpSt2, double **hpQ2, double **hr2, double **hq2, double *work_double, int N2_cond)
	{

	const int bs = D_MR;
	const int ncl = D_NCL;

	int pnx = (nx+bs-1)/bs*bs;
	int cnx = (nx+ncl-1)/ncl*ncl;

	// find problem size
	int N1 = N/N2; // floor
	int R1 = N - N2*N1; // the first r1 stages are of size N1+1
	int M1 = R1>0 ? N1+1 : N1;
	int T1;

	int ii, jj, nn;
	int use_Gamma_0_Q = 0;
	if(N2_cond==0 && diag_Q==0)
		use_Gamma_0_Q = 1;

	double *(hpGamma_0[M1]);
	double *(hpGamma_0_Q[M1]);
	double *(hpGamma_u[M1]);
	double *(hpGamma_u_Q[M1]);
	double *(hpGamma_u_Q_A[M1]);
	double *(hGamma_b[M1]);
	double *(hGamma_b_q[M1]);
	double *(hpL[M1+1]);
	double *work;

	double *ptr;
	ptr = work_double;

	for(jj=0; jj<M1; jj++)
		{
		hpGamma_0[jj] = ptr;
		ptr += pnx*cnx;
		}

	for(jj=0; jj<M1; jj++)
		{
		hpGamma_0_Q[jj] = ptr;
		ptr += pnx*cnx;
		}

	for(jj=0; jj<M1; jj++)
		{
		hpGamma_u[jj] = ptr;
		ptr += ((jj+1)*nu+bs-1)/bs*bs*cnx;
		}

	for(jj=0; jj<M1; jj++)
		{
		hpGamma_u_Q[jj] = ptr;
		ptr += ((jj+1)*nu+bs-1)/bs*bs*cnx;
		}

	for(jj=0; jj<M1; jj++)
		{
		hpGamma_u_Q_A[jj] = ptr;
		ptr += ((jj+1)*nu+bs-1)/bs*bs*cnx;
		}
	
	for(jj=0; jj<=M1; jj++)
		{
		hpL[jj] = ptr;
		if(diag_Q)
			ptr += pnx;
		else
			ptr += pnx*cnx;
		}

	for(jj=0; jj<M1; jj++)
		{
		hGamma_b[jj] = ptr;
		ptr += pnx;
		}

	for(jj=0; jj<M1; jj++)
		{
		hGamma_b_q[jj] = ptr;
		ptr += pnx;
		}
	
	work = ptr;
	ptr += pnx;



	nn = 0;
	for(jj=0; jj<N2; jj++)
		{

		T1 = jj<R1 ? M1 : N1;

		nx2[jj] = nx;
		nu2[jj] = T1*nu;


		// condense dynamic system
		d_cond_A(T1, nx, nu, hpA+nn, 1, hpGamma_0, hpA2[jj]);

		d_cond_B(T1, nx, nu, hpA+nn, hpBt+nn, 1, hpGamma_u, hpB2[jj]);

		d_cond_b(T1, nx, nu, hpA+nn, hb+nn, 1, hGamma_b, hb2[jj]);


		// condense cost function
		d_cond_Q(T1, nx, nu, hpA+nn, diag_Q, 0, hpQ+nn, hpL, 0, hpGamma_0, hpGamma_0_Q, hpQ2[jj], work);
		
		d_cond_R(T1, nx, nu, N2_cond, hpA+nn, hpAt+nn, hpBt+nn, diag_Q, 0, hpQ+nn, hpL, nzero_S, hpS+nn, hpR+nn, 0, hpGamma_u, hpGamma_u_Q, hpGamma_u_Q_A, hpR2[jj]);

		d_cond_St(T1, nx, nu, nzero_S, hpS+nn, 0, hpGamma_0, use_Gamma_0_Q, hpGamma_0_Q, hpGamma_u_Q, hpSt2[jj]);

		d_cond_q(T1, nx, nu, hpA+nn, hb+nn, diag_Q, 0, hpQ+nn, hq+nn, hpGamma_0, 0, hGamma_b, 1, hGamma_b_q, hq2[jj]);

		d_cond_r(T1, nx, nu, hpA+nn, hb+nn, diag_Q, 0, hpQ+nn, nzero_S, hpS+nn, hq+nn, hr+nn, hpGamma_u, 0, hGamma_b, 0, hGamma_b_q, hr2[jj]);


		// increment stage counter
		nn += T1;

		}
	
	}
#endif
