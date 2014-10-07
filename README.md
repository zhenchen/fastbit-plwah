fastbit-plwah
=============

The design space in bitmap index compression is still a fruitful unknown frontier and worth to explore.

We impelement PLWAH(Position list word aligned hybrid) encoding in FastBit as a benchmark for testing our newly designed Bitmap Index compression schemes named SECOMPAX and many others.

SECOMPAX is a new bitmap index encoding algorithm, it is a descent of COMPAX with new designed codebook. SECOMPAX is the abbreviation of Scope-Extended COMPressed Adaptive indeX. In Chinese, it named "展域压缩自适应索引."

For details in PLWAH, please refer to the work as follows:

[1] F. Deli`ege and T. B. Pedersen. Position list word aligned hybrid: optimizing space and performance for compressed bitmaps. Proc. of the 13th Int. Conf. on Extending Database Technology, EDBT'2010, 2010.

If you feel interested in SECOMPAX, please cite it as follows:

[1] Wen, Yuhao, et al. "SECOMPAX: A bitmap index compression algorithm." Computer Communication and Networks (ICCCN), 2014 23rd International Conference on. IEEE, 2014.

please visit URL: http://ieeexplore.ieee.org/xpl/articleDetails.jsp?arnumber=6911838.
