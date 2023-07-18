import matplotlib.pyplot as plt
from statistics import median

makePlotsToo = True

def makeHisto(experimentName, nb):
    print("{}. {}".format(nb, experimentName))
    fileSmall = open("./log/{}_NA.log".format(experimentName), 'r')
    fileBig = open("./log/{}_A.log".format(experimentName), 'r')

    small = [int(x) for x in fileSmall.read().split()]
    big = [int(x) for x in fileBig.read().split()]

    if median(small) == 0:
        print("\tSkipped this experiment because the L1 measurement set is LLC-congruent\n")
        return

    mB, mS = median(big), median(small)
    if (mB < mS):
        small, big = big, small
        ratio = mS/mB 
    else: 
        ratio = mB/mS 

    if makePlotsToo:
        lowRange = int(0.80*min(small))
        highRange = int(1.20*max(big))
        if experimentName != "L1_LLC":
            bins = [i for i in range(lowRange, highRange, (highRange-lowRange)//100)]
        else:
            bins = [i for i in range(lowRange, highRange, (highRange-lowRange)//10)]

        plt.figure(figsize=(8,4))
        plt.hist([small, big], bins, label=['no access', 'access'])
        plt.xlabel('Number of cycles')
        plt.ylabel('Occurrence')
        plt.legend(loc='upper right')
        if experimentName == "L1_EV":
            plt.title("Amplifying L1 eviction (=leaky.page)")
        elif experimentName == "L1_ORD":
            plt.title("Amplifying L1 ordering")
        else:
            plt.title("Amplifying LLC eviction")
        plt.savefig("./log/{}.png".format(experimentName))
    

    fileMetaData = open("./log/metadata.log", 'r')
    traversalsL1, traversalsLLC, LLC_congruent, hugePages, distance, refresh = [int(l) for l in fileMetaData.read().split()]
    fileData = open("./log/results_{}.log".format(experimentName), 'a')
    if (nb >= 5): # LLC amp
        fileData.write("{},{},{},{},{},{},{}\n".format(traversalsLLC, mS, mB, LLC_congruent, hugePages, distance, refresh))
    else: # L1 amp
        fileData.write("{},{},{},{},{},{},{}\n".format(traversalsL1, mS, mB, LLC_congruent, hugePages, distance, refresh))

    fileSmall.close()
    fileBig.close()
    fileData.close()
    fileMetaData.close()

    print("\tRatio of medians is {:.2f}\n".format(ratio))

def main():

    print("\n#########\n L1 PLRU\n#########")
    makeHisto("L1_EV", 1)
    makeHisto("L1_ORD", 2)
    makeHisto("L1_INV", 3)
    makeHisto("LLC_EV", 4)
    makeHisto("LLC_ORD", 5)
    print("\n##########\n PREFETCH\n##########")
    makeHisto("LLC_PREF", 6)
    makeHisto("L1_LLC", 7)

    if makePlotsToo:
        print("\nSee ./log for histogram png files \n")


if __name__ == "__main__":
    main()