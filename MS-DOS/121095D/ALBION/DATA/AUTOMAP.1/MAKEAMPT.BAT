convebes amtl1.bbm amtl1.raw
convebes amtl2.bbm amtl2.raw
convebes amtr1.bbm amtr1.raw
convebes amtr2.bbm amtr2.raw
convebes ambl1.bbm ambl1.raw
convebes ambl2.bbm ambl2.raw
convebes ambr1.bbm ambr1.raw
convebes ambr2.bbm ambr2.raw

convebes amt1.bbm amt1.raw
convebes amt2.bbm amt2.raw
convebes amt3.bbm amt3.raw
convebes amt4.bbm amt4.raw

convebes amb1.bbm amb1.raw
convebes amb2.bbm amb2.raw
convebes amb3.bbm amb3.raw
convebes amb4.bbm amb4.raw

convebes aml1.bbm aml1.raw
convebes aml2.bbm aml2.raw
convebes aml3.bbm aml3.raw

convebes amr1.bbm amr1.raw
convebes amr2.bbm amr2.raw
convebes amr3.bbm amr3.raw

convebes amsurf1.bbm amsurf1.raw
convebes amsurf2.bbm amsurf2.raw
convebes amsurf3.bbm amsurf3.raw
convebes amsurf4.bbm amsurf4.raw
convebes amsurf5.bbm amsurf5.raw

copy /b amtl1.raw + amtl2.raw + amtr1.raw + amtr2.raw + ambl1.raw + ambl2.raw + ambr1.raw, autom01.tmp
copy /b ambr2.raw + amt1.raw + amt2.raw + amt3.raw + amt4.raw + amb1.raw + amb2.raw, autom02.tmp
copy /b amb3.raw + amb4.raw + aml1.raw + aml2.raw + aml3.raw + amr1.raw + amr2.raw, autom03.tmp
copy /b amr3.raw + amsurf1.raw + amsurf2.raw + amsurf3.raw + amsurf4.raw + amsurf5.raw, autom04.tmp

copy /b autom01.tmp + autom02.tmp + autom03.tmp + autom04.tmp, autopart

del autom01.tmp
del autom02.tmp
del autom03.tmp
del autom04.tmp

