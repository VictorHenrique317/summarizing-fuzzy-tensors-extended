BASIS_convert<-function(VEC){
  if(is.na(VEC[1])){
    temp<-""
  }else{
    temp<-VEC[1]
  }
  for (i in 2:length(VEC)) {
    if(is.na(VEC[i])){
      temp<-paste(temp,",",sep = "")
    }else{
      temp<-paste(temp,VEC[i],sep = ",")
    }
  }
  return(temp)
}

GENERATE_ORDER<-function(VEC,MAT=NULL){
  if(is.null(MAT)){
    DIM<-1
  }else{
    DIM<-nrow(MAT)
  }
  MAT1<-NULL
  if(length(VEC)==1){
    MAT1<-cbind(MAT,rep(VEC[1],DIM))
    return(MAT1)
  }else{
    for (i in 1:length(VEC)) {
      MAT2<-cbind(MAT,rep(VEC[i],DIM))
      MAT1<-rbind(MAT1,GENERATE_ORDER(VEC[-i],MAT2))
    }
    return(MAT1)
  }
}


POS<-function(VEC,n){
  temp<-which(VEC>1)
  if(length(temp)<n){
    return(0)
  }else{
    return(temp[order(VEC[temp],decreasing = T)[length(temp)%/%n]])
  }
}





FOLD<-function(TENS,n){
  if(is.null(dim(TENS))){
    return(POS(TENS,n))
  }else{
    TENS1<-apply(TENS,2:length(dim(TENS)),sum)
    test<-FOLD(TENS1,n+1)
    temp<-NULL
    for (i in 1:length(test)) {
      temp<-paste(temp,",test[",i,"]",sep = "")
    }
    C<-eval(parse(text = paste("POS(TENS[",temp,"],n)",sep = "")))
    return(c(C,test))
  }
}


TENS_update<-function(TENS,BASIS){
  VEC<-rep(",",length(dim(TENS)))
  VEC[is.na(BASIS)]<-paste("which(TENS[",BASIS_convert(BASIS),"]==1)",sep="")
  VEC<-paste(VEC,collapse = "")
  
  DIM<-1:length(dim(TENS))
  DIM<-DIM[!is.na(BASIS)]
  TENS<-eval(parse(text = paste("apply(TENS[",VEC,"],DIM,sum)",sep = "")))
  return(TENS)
}


TENS_update_vec<-function(TENS,ORDER,VEC){
  
  temp<-rep(",",length(dim(TENS)))
  temp[ORDER[1]]<-"which(VEC==1)"
  temp<-paste(temp,collapse = "")
  
  if(sum(VEC)==1){
    TENS<-eval(parse(text = paste("TENS[",temp,"]",sep = "")))
  }else{
    DIM<-1:length(dim(TENS))
    DIM<-DIM[-ORDER[1]]
    TENS<-eval(parse(text = paste("apply(TENS[",temp,"],DIM,sum)",sep = "")))
  }
  return(TENS)
}


BASIS_FIND<-function(TENS,ORDER=NULL,Thres=0.6){
  
  if(is.null(ORDER)){
    ORDER<-sample(1:length(dim(TENS)),length(dim(TENS)),replace = F)
  }
  
  ORDER_use<-ORDER
  LIST<-list()
  
  while (length(ORDER_use)>=2) {
    if(length(ORDER_use)>2){
      
      TEMP<-apply(TENS, ORDER_use[2:length(ORDER_use)], sum)
      temp<-rep(NA,length(dim(TENS)))
      temp[ORDER_use[-1]]<-FOLD(TEMP,2)
      
      if(all(temp[!is.na(temp)]>0)){
        
        VEC<-eval(parse(text = paste("TENS[",BASIS_convert(temp),"]",sep = "")))
        LIST[[length(LIST)+1]]<-VEC
        TENS<-TENS_update(TENS,temp)
        TENS<-array(as.numeric(TENS>(sum(VEC)*Thres)),dim=dim(TENS))
        
      }else{
        
        temp<-unique(as.numeric(TEMP))
        temp<-temp[order(temp,decreasing = T)]
        LOC<-which(TEMP==temp[1],arr.ind = T)
        
        if(nrow(LOC)>1){
          LOC<-LOC[1:2,]
        }else{
          LOC<-rbind(LOC,which(TEMP==temp[2],arr.ind = T)[1,])
        }
        
        LOC<-cbind(rep(NA,2),LOC)
        LOC[,ORDER_use]<-LOC
        VEC<-eval(parse(text = paste("TENS[",BASIS_convert(LOC[1,]),"]*TENS[",BASIS_convert(LOC[2,]),"]",sep = "")))
        
        if(sum(VEC)==0){
          for (k in (length(LIST)+1):length(ORDER)) {
            LIST[[k]]<-0
          }
          return(LIST)
        }else{
          LIST[[length(LIST)+1]]<-VEC
          TENS<-TENS_update_vec(TENS,ORDER_use,VEC)
          TENS<-array(as.numeric(TENS>(sum(VEC)*Thres)),dim=dim(TENS))
        }
      }
      
      ORDER_new<-ORDER_use[2:length(ORDER_use)]
      ORDER_new[ORDER_new>ORDER_use[1]]<-ORDER_new[ORDER_new>ORDER_use[1]]-1
      ORDER_use<-ORDER_new
      
    }else{
      TEMP<-apply(TENS, ORDER_use[2:length(ORDER_use)], sum)
      
      temp<-rep(NA,length(dim(TENS)))
      temp[ORDER_use[-1]]<-FOLD(TEMP,2)
      
      if(temp[!is.na(temp)]>0){
        VEC<-eval(parse(text = paste("TENS[",BASIS_convert(temp),"]",sep = "")))
        
        LIST[[length(LIST)+1]]<-VEC
        ORDER_use<-ORDER_use[-1]
        
        TENS<-TENS_update(TENS,temp)
        LIST[[length(LIST)+1]]<-as.numeric(TENS>(sum(VEC)*Thres))
      }else{
        temp<-unique(as.numeric(TEMP))
        temp<-temp[order(temp,decreasing = T)]
        LOC<-which(TEMP==temp[1],arr.ind = T)
        
        if(length(LOC)>1){
          LOC<-LOC[1:2]
        }else{
          LOC<-c(LOC,which(TEMP==temp[2],arr.ind = T)[1])
        }
        
        LOC<-cbind(rep(NA,2),LOC)
        LOC[,ORDER_use]<-LOC
        VEC<-eval(parse(text = paste("TENS[",BASIS_convert(LOC[1,]),"]*TENS[",BASIS_convert(LOC[2,]),"]",sep = "")))
        LIST[[length(LIST)+1]]<-VEC
        
        
        if(sum(VEC)==0){
          LIST[[length(LIST)+1]]<-rep(0,dim(TENS)[!is.na(LOC[1,])])
        }else if(sum(VEC)==1){
          if(ORDER_use[is.na(LOC[1,])]==1){
            LIST[[length(LIST)+1]]<-TENS[,which(VEC==1)]
          }else{
            LIST[[length(LIST)+1]]<-TENS[which(VEC==1),]
          }
        }else{
          if(ORDER_use[is.na(LOC[1,])]==1){
            TENS<-apply(TENS[,which(VEC==1)],1,sum)
            LIST[[length(LIST)+1]]<-as.numeric(TENS>(sum(VEC)*Thres))
          }else{
            TENS<-apply(TENS[which(VEC==1),],2,sum)
            LIST[[length(LIST)+1]]<-as.numeric(TENS>(sum(VEC)*Thres))
          }
        }
        
        ORDER_use<-ORDER_use[-1]
      }
    }
  }
  
  LIST[ORDER]<-LIST
  
  return(LIST)
}





MEASURE<-function(TENS,BASIS){
  if(all(lapply(BASIS,sum)>0)){
    ALL<-sum(TENS)
    temp<-"which(BASIS[[1]]==1)"
    for (i in 2:length(BASIS)) {
      temp<-paste(temp,",which(BASIS[[",i,"]]==1)",sep = "")
    }
    SUM<-eval(parse(text = paste("sum(TENS[",temp,"])",sep = "")))
    PROD<-prod(unlist(lapply(BASIS, sum)))
    NUM1<-SUM/PROD
    NUM2<-SUM/ALL
    NUM3<-SUM*2-PROD
    return(c(NUM1,NUM2,NUM3))
  }else{
    return(c(-Inf,-Inf,-Inf))
  }
}



GEO_SEG<-function(TENS,Thres,Exhausive=F){
  
  DIM<-length(dim(TENS))
  ORDER<-matrix(0,nrow = DIM,ncol = DIM)
  if(Exhausive){
    ORDER<-GENERATE_ORDER(1:DIM)
  }else{
    for(i in 1:DIM){
      ORDER[i,]<-sample(1:DIM,DIM,replace = F)
    }
  }
  
  C<-rep(0,3)
  BASIS<-NULL
  
  for (i in 1:nrow(ORDER)) {
    BASIS_temp<-BASIS_FIND(TENS,ORDER[i,],Thres)
    C_temp<-MEASURE(TENS,BASIS_temp)
    if(C_temp[3]>C[3]){
      BASIS<-BASIS_temp
      C<-C_temp
    }
  }
  
  return(BASIS)
}






GETF_CP<-function(TENS,Thres=0.6,B_num=20,COVER=0.9,Exhausive=F){
  
  BASIS_ALL<-list()
  for (i in 1:length(dim(TENS))) {
    BASIS_ALL[[i]]<-matrix(0,nrow = dim(TENS)[i],ncol = 0)
  }
  
  B_now<-0
  Cover_now<-0
  SUM<-sum(TENS)
  SUM_now<-0
  
  while(B_now<B_num&Cover_now<COVER){
    BASIS<-GEO_SEG(TENS,Thres,Exhausive)
    if(!is.null(BASIS)){
      temp<-"which(BASIS[[1]]==1)"
      for (i in 2:length(BASIS)) {
        temp<-paste(temp,paste("which(BASIS[[",i,"]]==1)",sep = ""),sep = ",")
      }
      SUM_now<-SUM_now+eval(parse(text = paste("sum(TENS[",temp,"])",sep = "")))
      eval(parse(text=paste("TENS[",temp,"]<-0",sep = "")))
      for (i in 1:length(BASIS)) {
        BASIS_ALL[[i]]<-cbind(BASIS_ALL[[i]],BASIS[[i]])
      }
    }else{
      break
    }
    Cover_now<-SUM_now/SUM
    #print(Cover_now)
    B_now<-B_now+1
  }
  
  return(BASIS_ALL)
}





Reconstruct_error<-function(TENS,BASIS){
  # print(class(BASIS[1]))
  # print(class(BASIS[[1]]))
  # print(dim(BASIS[[1]]))
  # print(BASIS[[1]])
  if(is.null(dim(BASIS[[1]]))){ # se a dimensão do primeiro fator for nula 
    TENS_compare<-BASIS[[1]]
    for (i in 2:length(BASIS)) {
      TENS_compare<-TENS_compare%o%BASIS[[i]]
    }
  }else{ # parte importante
    # print("Parte importante ===>")
    TENS_compare<-0*TENS # monta matriz nula a partir do tamanho do TENS
    
    for (j in 1:ncol(BASIS[[1]])) { # j assume os valores de 1 até o número de colunas do primeiro fator
      TENS_temp<-BASIS[[1]][,j] # pega todas as linhas da j-ésima coluna (pega a j-ésima coluna)
      for (i in 2:length(BASIS)) { # i assume os valores de 2 até o numero de fatores
        TENS_temp<-TENS_temp%o%BASIS[[i]][,j]
      }
      
      # TENS_temp = Factor matrix
      # TENS_temp da a posição dos padrões ???
      print(TENS_temp)
      TENS_compare<-TENS_compare+TENS_temp
     
    }
  }
  
  # print(TENS_compare)
  TENS_compare<-array(as.numeric(TENS_compare>0),dim=dim(TENS_compare))
  # print(TENS_compare)
  # o algoritmo calcula o outer product da coluna j de todos os fatores e soma na matriz nula
  # depois muda para a coluna j+1 e calcula novamente e soma na matriz resultante (iterativamente)
  return(sum(abs(TENS-TENS_compare))/prod(dim(TENS)))
}






Tensor_Simulate<-function(Dims,pattern=5,density=0.2,Noise=0.01){
  P_MAT<-list()
  for (i in 1:length(Dims)) {
    P_MAT[[i]]<-matrix(rbinom(pattern*Dims[i],1,0.2),ncol = pattern,nrow = Dims[i])
  }
  TENS<-array(0,dim = Dims)
  for (i in 1:pattern) {
    temp<-P_MAT[[1]][,i]
    for (j in 2:length(Dims)) {
      temp<-temp%o%P_MAT[[j]][,i]
    }
    TENS<-TENS+temp
  }
  TENS<-array(as.numeric(TENS>0),dim = Dims)
  TENS<-abs(TENS-array(rbinom(prod(Dims),1,Noise),dim=Dims))
  return(TENS)
}

Get_Patterns<-function(TENS,BASIS, experiment_path){
  file.create(experiment_path)
  experiment_file <- file(experiment_path)
  Patterns <- list()
  if (length(BASIS[[1]]) == 0){
    return(Patterns)
  }

  if(is.null(dim(BASIS[[1]]))){
    TENS_temp<-BASIS[[1]]
    for (i in 2:length(BASIS)) {
      TENS_temp<-TENS_temp%o%BASIS[[i]]
    }

    Patterns[[j]] <- array(as.numeric(TENS_temp>0),dim=dim(TENS_temp))
  }else{
    cat("Starting to extract", ncol(BASIS[[1]]),"patterns\n")
    TENS_compare<-0*TENS
    i <- 0
    for (j in 1:ncol(BASIS[[1]])) {
        cat("\r", j, "done")
        TENS_temp<-BASIS[[1]][,j]
      for (i in 2:length(BASIS)) {
        TENS_temp<-TENS_temp%o%BASIS[[i]][,j]
      }

        # BEFORE
        # Pattern <- array(as.numeric(TENS_temp>0),dim=dim(TENS_temp))
        # # pattern processing into string
        # one_cells_indices = which(Pattern==1, arr.ind=TRUE)
        # index_string <- ""

        # for (j in 1:ncol(one_cells_indices)){
        #     # cat("\nj", j)
        #     for (i in 1:nrow(one_cells_indices)){
        #         index_string <- paste(index_string, one_cells_indices[i,j] - 1, ",")
        #         index_string  <- gsub(" ", "", index_string)
        #     }
        #     index_string <- substring(index_string,1, nchar(index_string)-1)
        #     index_string <- paste(index_string, "-")
        # }

        # index_string <- substring(index_string,1, nchar(index_string)-2)

        #AFTER
        Pattern <- array(as.numeric(TENS_temp>0),dim=dim(TENS_temp))
        one_cells_indices = which(Pattern==1, arr.ind=TRUE)
        index_string <- apply(one_cells_indices, 2, function(x) paste(x-1, collapse=","))
        index_string <- paste(index_string, collapse="-")


        indices_as_strings <- strsplit(index_string, "-")[[1]]
        final_string <- ""
        for (tuple in indices_as_strings){
            temp_list <- as.list(strsplit(tuple, ","))[[1]]
            temp_list <- unique(temp_list)
            temp_list <- paste(temp_list, collapse = ", ")
            temp_list <- gsub(" ", "", temp_list)
            final_string <- paste(final_string, temp_list)
        }
        # final_string <- paste(final_string, "1")
        # pattern processing into string

#         writeLines(c(final_string), experiment_file)
        write(final_string, file=experiment_path, append=TRUE)
    }
  }
  close(experiment_file)
  print("Finished")
  return(ncol(BASIS[[1]]))
}


#################################### GETF ALGORITHM  ABOVE  ######################################################

args = commandArgs()

translated_tensor_path = args[6]

noise_endurance = as.double(args[7])
max_pattern_number = as.integer(args[8])
current_iteration_folder = args[9]
current_experiment = args[10]
experiment_path = args[11]
library("reticulate")

print("==== GETF ====")
np <- import("numpy")
tensor <- np$load(translated_tensor_path)

start_time <- Sys.time()
print("Starting algorithm")
Factors<-GETF_CP(TENS = tensor, Thres = 0.6, B_num = max_pattern_number, COVER = 0.9, Exhausive = F)
print("GETF finished with success")
# saveRDS(Factors, file="../temp/getf_factors.rds")
# print("Factors saved with success")
end_time <- Sys.time()
time_spent <- difftime(end_time, start_time, units = "secs")[[1]]
print("Time spent:")
print(time_spent)

# Factors <- readRDS("../temp/getf_factors.rds") # TEMPORARY
nb_patterns <- Get_Patterns(tensor, Factors, experiment_path)

log_file_path = paste(current_iteration_folder, "/output/", current_experiment, "/logs/getf.log")
log_file_path = gsub(" ", "", log_file_path)

filler <- "filler:0"
pattern_nb <- paste("Nb of patterns:", nb_patterns)
time_spent <- paste("Run time:", time_spent)

file.create(log_file_path)
log_file <- file(log_file_path)
writeLines(c(filler, filler, filler, filler, pattern_nb, time_spent), log_file)
print("Log file created!")
close(log_file)
