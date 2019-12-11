#!/usr/bin/Rscript --vanilla 
schedulability_impact <- function()
{
    path <- c("./schedulability_result_files/")
    
    
#     period <- c("homogeneous", "heterogeneous")
#     utili <- c("light", "medium")
#     cs <- c("short", "medium", "long")
#     nres <- c("1", "2", "4", "8", "16")
#     pacc <- c("0.1", "0.25", "0.5")
    
    period <- c("heterogeneous")
    utili <- c("light")
    cs <- c("long")
    nres <- c("8")
    pacc <- c("0.5")
    
    base <- c("./output_pdf/")
    plot_colors <- c("blue", "red", "grey20", "darkblue","darkred","darkgreen", "rosybrown3", "darkgoldenrod3", "cyan3")
    
    for (i in utili) {
     for(j in period) 
       for(k in cs)
         for(l in nres)
           for(m in pacc) {
            file_tmp <- paste(path, paste(i,j,k,l,m, sep="_"), sep="")
            file <- paste(file_tmp, ".txt", sep="")
            output_file <- paste(file_tmp, ".pdf", sep="")
            output_file <- gsub(path, base, output_file, fixed = T)
            pdf(file=output_file, height=5, width=8)
            data <- read.table(file, header = TRUE)
            
            t <- plot(data$PIP, type="b", col=plot_colors[1], ylim=c(0,1), 
                      xlab="Task set utilization cap (before adding the protocols overhead)", ylab="Ratio of schedulable task sets", 
                      cex=2, lwd=2, xaxt="n", lty=1, pch=1, panel.first = abline(h = seq(0, 1.0, 0.1), v = seq(0, 10, 1), lty=2, lwd=.2, col="lightgrey"))
            #title(main="Teste", col.main="black", font.main=4, cex.main=1.2)
            axis(1, labels=c(data$Utili), at = 1:10, cex.axis=1)
            lines(data$PIPO, type="b", lty=2, lwd=2, cex=2, col=plot_colors[2], pch=2)
            lines(data$PCP, type="b", lty=3, lwd=2, cex=2, col=plot_colors[3], pch=3)
            lines(data$PCPO, type="b", lty=4, lwd=2, cex=2, col=plot_colors[4], pch=4)
            lines(data$IPCP, type="b", lty=5, lwd=2, cex=2, col=plot_colors[5], pch=5)
            lines(data$IPCPO, type="b", lty=6, lwd=2, cex=2, col=plot_colors[6], pch=6)
            lines(data$SRP, type="b", lty=7, lwd=2, cex=2, col=plot_colors[7], pch=7)
            lines(data$SRPO, type="b", lty=8, lwd=2, cex=2, col=plot_colors[8], pch=8)
            
            legend('topright','groups', c("PIP","PIP Ov","PCP","PCP Ov",
                                     "IPCP", "IPCP Ov", "SRP", "SRP Ov"), 
                   cex=1, col=plot_colors, lty=1:8, pch=1:8, pt.lwd=4, pt.cex=1.6, merge = FALSE, 
                   ncol=2, box.lwd = par("lwd"), box.lty = par("lty"),  box.col = par("fg"), bg='white')

            #print(file)
            #print(output_file)
            dev.off()
            
            #convert to png
            system(paste("convert", output_file, gsub("pdf", "png", output_file)))
           }
    }
}

schedulability_impact()