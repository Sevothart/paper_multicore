plot_run_time_overhead_graphs <- function()
{
  run_time_overhead <- read.table("./run_time_overhead_eposmote.txt", header = TRUE)
  
  plot_colors <- c("blue", "red", "grey20", "darkblue","darkred","darkgreen", "rosybrown3", "darkgoldenrod3", "cyan3")
  pdf(file="run_time_overhead_eposmote.pdf", height=5, width=8)
  t <- plot(run_time_overhead$PIP.p, type="b", col=plot_colors[1], ylim=c(0,27), 
            xlab="Number of Waiting Tasks", ylab="Run-time Overhead (in us)", 
            cex=2, lwd=2, xaxt="n", lty=1, pch=1, 
            panel.first = abline(h = seq(0, 25, 5), v = seq(0, 20, 1), lty=2, lwd=.2, col="lightgrey"))
  title(main="Run-time Overhead of the Implemented Protocols", col.main="black", font.main=4, cex.main=1.2)
  axis(1, lab=c(run_time_overhead$Tasks), 
       at=c(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20), cex.axis=1.5)
  lines(run_time_overhead$PIP.v, type="b", lty=2, lwd=2, cex=2, col=plot_colors[2], pch=2)
  lines(run_time_overhead$PCP.p, type="b", lty=3, lwd=2, cex=2, col=plot_colors[3], pch=3)
  lines(run_time_overhead$PCP.v, type="b", lty=4, lwd=2, cex=2, col=plot_colors[4], pch=4)
  lines(run_time_overhead$IPCP.p, type="b", lty=5, lwd=2, cex=2, col=plot_colors[5], pch=5)
  lines(run_time_overhead$IPCP.v, type="b", lty=6, lwd=2, cex=2, col=plot_colors[6], pch=6)
  lines(run_time_overhead$SRP.p, type="b", lty=7, lwd=2, cex=2, col=plot_colors[7], pch=7)
  lines(run_time_overhead$SRP.v, type="b", lty=8, lwd=2, cex=2, col=plot_colors[8], pch=8)
  legend('top','groups', c("PIP p","PIP v","PCP p","PCP v",
                   "IPCP p", "IPCP v", "SRP p", "SRP v"), 
         cex=1, col=plot_colors, lty=1:8, pch=1:8, pt.lwd=4, pt.cex=1.6, merge = FALSE, 
         box.lwd = par("lwd"), box.lty = par("lty"),  box.col = par("fg"), bg='white',ncol=4)
  dev.off()
}


plot_run_time_overhead_graphs()

